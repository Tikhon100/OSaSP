#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>
#include <sys/stat.h>

int CURRENT_NUM_RECORDS = 10;
#define RECORD_SIZE sizeof(struct record_s)

struct record_s
{
    char name[80];
    char address[80];
    int semester;
};

void lock_record(int fd, int rec_no, int lock_type)
{
    struct flock lock;
    lock.l_type = lock_type;
    lock.l_whence = SEEK_SET;
    lock.l_start = rec_no * RECORD_SIZE;
    lock.l_len = RECORD_SIZE;
    fcntl(fd, F_SETLKW, &lock);
}

void unlock_record(int fd, int rec_no)
{
    lock_record(fd, rec_no, F_UNLCK);
}

struct record_s get_record(int fd, int rec_no)
{
    struct record_s record;
    lseek(fd, rec_no * RECORD_SIZE, SEEK_SET);
    read(fd, &record, RECORD_SIZE);
    return record;
}

void put_record(int fd, struct record_s record, int rec_no)
{
    lseek(fd, rec_no * RECORD_SIZE, SEEK_SET);
    write(fd, &record, RECORD_SIZE);
}

void generate_records(int fd)
{
    struct record_s record;
    int i;
    const char *names[] = {"Валера", "Виталя", "Никита", "Алешка", "Стасик", "Галя", "Света", "Воропаев", "Тихон", "Валентин"};
    const char *addresses[] = {"Гомель", "Минск", "Калинковичи", "Мозырь", "Киев", "Зона 51", "Борисов", "Жлобин"};

    srand(time(NULL));

    for (i = 0; i < CURRENT_NUM_RECORDS; i++)
    {
        strncpy(record.name, names[rand() % 10], sizeof(record.name) - 1);
        record.name[sizeof(record.name) - 1] = '\0';
        strncpy(record.address, addresses[rand() % 8], sizeof(record.address) - 1);
        record.address[sizeof(record.address) - 1] = '\0';
        record.semester = rand() % 8 + 1;
        put_record(fd, record, i);
    }
}

void list_records(int fd)
{
    struct record_s record;
    int i;
    for (i = 0; i < CURRENT_NUM_RECORDS; i++)
    {
        record = get_record(fd, i);
        printf("%d. %s, %s, Semester %d\n", i + 1, record.name, record.address, record.semester);
    }
}

int main(int argc, char *argv[])
{
    int fd, rec_no, choice;
    struct record_s record, record_new, record_sav, buffer_record;

    if ((fd = open("records.dat", O_RDWR)) == -1)
    {
        // Файл не существует, создаем его
        if ((fd = open("records.dat", O_RDWR | O_CREAT, 0644)) == -1)
        {
            perror("open");
            exit(1);
        }
        generate_records(fd); // Генерируем начальные записи
    }
    else
    {
        struct stat file_stat;
        if (fstat(fd, &file_stat) == -1)
        {
            perror("fstat");
            exit(1);
        }
        CURRENT_NUM_RECORDS = file_stat.st_size / RECORD_SIZE;
    }

    while (1)
    {
        printf("\n1. List Records\n2. Get Record\n3. Modify Record\n4. Add Record\n5. Exit\nEnter choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            list_records(fd);
            break;
        case 2:
            printf("Enter record number: ");
            scanf("%d", &rec_no);
            record = get_record(fd, rec_no - 1);
            printf("Name: %s\nAddress: %s\nSemester: %d\n", record.name, record.address, record.semester);
            break;
        case 3:
            printf("Enter record number: ");
            scanf("%d", &rec_no);
            rec_no--;

        again:

            record_sav = get_record(fd, rec_no);

            memcpy(&buffer_record, &record_sav, RECORD_SIZE);

            printf("Name: %s\nAddress: %s\nSemester: %d\n", record_sav.name, record_sav.address, record_sav.semester);
            lock_record(fd, rec_no, F_WRLCK);

            printf("Enter new name: ");
            scanf(" %79s", record.name);
            printf("Enter new address: ");
            scanf(" %79s", record.address);
            printf("Enter new semester: ");
            scanf("%d", &record.semester);

            record_new = get_record(fd, rec_no);
            if (memcmp(&record_new, &buffer_record, RECORD_SIZE) != 0)
            {
                unlock_record(fd, rec_no);
                printf("Record was modified by another process. Retrying...\n");
                goto again;
            }
            put_record(fd, record, rec_no);
            unlock_record(fd, rec_no);
            break;
        case 4:
            printf("Enter name: ");
            scanf(" %79s", record.name);
            printf("Enter address: ");
            scanf(" %79s", record.address);
            printf("Enter semester: ");
            scanf("%d", &record.semester);
            put_record(fd, record, CURRENT_NUM_RECORDS);
            CURRENT_NUM_RECORDS++;
            break;
        case 5:
            close(fd);
            exit(0);
        default:
            printf("Invalid choice\n");
        }
    }

    return 0;
}