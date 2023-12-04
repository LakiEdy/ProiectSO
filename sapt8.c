#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <libgen.h>
 
char stats[1024];
 
// Funcție pentru a verifica dacă este un fișier BMP
int is_bmp_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    return (ext != NULL && strcmp(ext, ".bmp") == 0);
}
 
struct bmp_info_header {
    int width;
    int height;
} info_header;
 
 
char* extract_filename(const char *path)
{
    char *filename = strdup(path);
    char *basename_result = basename(filename);
    char *result = strdup(basename_result);
    free(filename);
    return result;
}
 
void get_bmp_info(const char *filename) {
    // Deschidem fisierul BMP pentru citire
    int read_fd = open(filename, O_RDONLY);
    if (read_fd == -1) {
        perror("Eroare la deschiderea fisierului bmp pentru citire");
        return;
    }
 
    // Citim header-ul BMP
    char header[54];
    if (read(read_fd, header, sizeof(header)) != sizeof(header)) {
        perror("Eroare la citirea header-ului BMP");
        close(read_fd);
        return;
    }
 
     // Setăm poziția curentă la începutul header-ului BMP
    if (lseek(read_fd, 18, SEEK_SET) == -1) {
        perror("Eroare la setarea poziției curente pentru citire");
        close(read_fd);
        return;
    }
 
    // Citim inalțimea și lățimea imaginii BMP
    if (read(read_fd, &info_header, sizeof(struct bmp_info_header)) != sizeof(struct bmp_info_header)) {
        perror("Eroare la citirea inaltimei si latimii BMP");
        close(read_fd);
        return;
    }
 
    // Deschidem fisierul BMP pentru scriere
    int write_fd = open("output.bmp", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (write_fd == -1) {
        perror("Eroare la deschiderea fisierului bmp pentru scriere");
        close(read_fd);
        return;
    }
 
    // Scriem header-ul BMP in fisierul de iesire
    if (write(write_fd, header, sizeof(header)) != sizeof(header)) {
        perror("Eroare la scrierea header-ului BMP");
        close(read_fd);
        close(write_fd);
        return;
    }
 
    // Setăm poziția curentă la începutul datelor imaginii BMP
    if (lseek(read_fd, *(int*)&header[10], SEEK_SET) == -1) {
        perror("Eroare la setarea poziției curente pentru citire");
        close(read_fd);
        close(write_fd);
        return;
    }
 
    int width = info_header.width;
    int height = info_header.height;
 
    int pixel_data_size = width * height;
 
 
    unsigned char pixel[3];
    // Citim, procesam pixelii si scriem in fisierul de iesire
    for (int i = 0; i < pixel_data_size; i++) {
        if (read(read_fd, pixel, sizeof(pixel)) != sizeof(pixel)) {
            perror("Eroare la citirea pixelilor BMP\n");
            close(read_fd);
            close(write_fd);
            return;
        }
 
        // Calculam intensitatea tonurilor de gri
        unsigned char P_gri = (pixel[2] + pixel[1] + pixel[0]) / 3;
 
        // Setam cele 3 valori cu valoarea P_gri
        pixel[0] = P_gri;
        pixel[1] = P_gri;
        pixel[2] = P_gri;
 
        // Scriem noile valori in fisierul de iesire
        if (write(write_fd, pixel, sizeof(pixel)) == -1) {
            perror("Eroare la scrierea pixelilor BMP\n");
            close(read_fd);
            close(write_fd);
            return;
        }
    }
 
    // Închidem ambele fisiere
    close(read_fd);
    close(write_fd);
}
 
 
 
void process_file(const char *filename, const char *output_dir)
{
    DIR *dir;
    if((dir = opendir(output_dir)) == NULL)
    {
        perror("Eroare deschidere director scriere");
        exit(1);
    }
    pid_t pid = fork();
    int status;
    int nr_linii = 0;
    // Obținem dimensiunea fișierului folosind funcția stat
    struct stat file_stat;
    if (lstat(filename, &file_stat) == -1) {
        perror("Eroare la obtinerea informatiilor despre fisier");
        return;
    }
 
    // Obținem identificatorul utilizatorului și timpul ultimei modificări
    char time_string[20];
    if(localtime(&file_stat.st_mtime) == NULL)
    {
        perror("Eroare obtinre timp modificare");
        return;
    }
    strftime(time_string, sizeof(time_string), "%d.%m.%Y", localtime(&file_stat.st_mtime));
 
    char permissions_user[4];
    char permissions_group[4];
    char permissions_other[4];
    sprintf(permissions_user, "%c%c%c",
            (file_stat.st_mode & S_IRUSR) ? 'R' : '-',
            (file_stat.st_mode & S_IWUSR) ? 'W' : '-',
            (file_stat.st_mode & S_IXUSR) ? 'X' : '-');
 
    sprintf(permissions_group, "%c%c%c",
            (file_stat.st_mode & S_IRGRP) ? 'R' : '-',
            (file_stat.st_mode & S_IWGRP) ? 'W' : '-',
            (file_stat.st_mode & S_IXGRP) ? 'X' : '-');
 
    sprintf(permissions_other, "%c%c%c",
            (file_stat.st_mode & S_IROTH) ? 'R' : '-',
            (file_stat.st_mode & S_IWOTH) ? 'W' : '-',
            (file_stat.st_mode & S_IXOTH) ? 'X' : '-');
 
 
    char *base = extract_filename(filename);
    char output_filename[1024];
    sprintf(output_filename, "%s/statistica_%s.txt", output_dir, base);
    if(pid < 0)
    {
        perror("Eroare fork\n");
        exit(-1);
    }else if(pid == 0)
    {
        if (S_ISLNK(file_stat.st_mode)) {
            // Deschidem fisierul statistica_link.txt pentru scriere
            int output_link = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (output_link == -1) {
                perror("Eroare la crearea fisierului statistica.txt");
                return;
            }
            char target[1024];
            struct stat leg_stat;
            if (stat(filename, &leg_stat) == -1) {
                perror("Eroare la obtinerea informatiilor despre fisier");
                return;
            }
 
            ssize_t len = readlink(filename, target, sizeof(target) - 1);
            if (len != -1) {
                target[len] = '\0';
                sprintf(stats, "nume legatura: %s\ndimensiune legatura: %ld\ndimensiune fisier target: %ld\ndrepturi de acces user legatura: %s\ndrepturi de acces grup legatura: %s\ndrepturi de acces altii legatura: %s\n", filename, file_stat.st_size, leg_stat.st_size, permissions_user, permissions_group, permissions_other);
            }
            nr_linii = write(output_link, stats, strlen(stats));
            if (nr_linii == -1) {
                perror("Eroare scriere in fisier de statistici");
            }
            close(output_link);
        }else if (S_ISREG(file_stat.st_mode))
        {
            // Verificăm dacă fișierul are extensia ".bmp"
            if (is_bmp_file(filename)) {
                pid_t pid_bmp = fork();
                if(pid_bmp == -1)
                {
                    perror("Eroare la crearea procesului pt imaginea BMP\n");
                    exit(-1);
                }else if (pid_bmp == 0){//proces fiu
                    get_bmp_info(filename);
                }
                else {
                    int status;
                    waitpid(pid_bmp, &status, 0);
                    printf("S-a incheiat procesul pentru imaginea BMP cu pid-ul %d si codul %d\n", pid_bmp, WEXITSTATUS(status));
                }
                // Deschidem fisierul statistica_bmp.txt pentru scriere
                int output_bmp = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (output_bmp == -1) {
                    perror("Eroare la crearea fisierului statistica.txt");
                    return;
                }
                sprintf(stats, "nume fisier: %s\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n", filename,  (long)file_stat.st_size, file_stat.st_uid, time_string, file_stat.st_nlink, permissions_user, permissions_group, permissions_other);
                nr_linii = write(output_bmp, stats, strlen(stats));
                if ( nr_linii == -1) {
                    perror("Eroare scriere in fisier de statistici");
                }
                close(output_bmp);
            }
            else {
                // Deschidem fisierul statistica_fisier.txt pentru scriere
                int output_fisier = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (output_fisier == -1) {
                    perror("Eroare la crearea fisierului statistica.txt");
                    return;
                }
                sprintf(stats, "nume fisier: %s\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n", filename, file_stat.st_size, file_stat.st_uid, time_string, file_stat.st_nlink, permissions_user, permissions_group, permissions_other);
                nr_linii = write(output_fisier, stats, strlen(stats));
                if (nr_linii == -1) {
                    perror("Eroare scriere in fisier de statistici");
                }
                close(output_fisier);
            }
        }else if (S_ISDIR(file_stat.st_mode))
        {
            // Deschidem fisierul statistica_folder.txt pentru scriere
            int output_folder = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (output_folder == -1) {
                perror("Eroare la crearea fisierului statistica.txt");
                return;
            }
            sprintf(stats, "nume director: %s\nidentificatorul utilizatorului: %d\ndrepturi de acces user: %s\ndrepturi de acces grup: %s\ndrepturi de acces altii: %s\n", filename, file_stat.st_uid, permissions_user, permissions_group, permissions_other);
            nr_linii = write(output_folder, stats, strlen(stats));
            if (nr_linii == -1) {
                perror("Eroare scriere in fisier de statistici");
            }
            close(output_folder);
        }else{
            //Nu se scrie nimic 
        }
        exit(nr_linii);
    }else {
        //daca e parinte
        pid = wait(&status);
        if(WIFEXITED(status))
        {
            printf("Child with id = %d exited with status code %d\n", pid, WEXITSTATUS(status));
        }
    }
    free(base);            
    closedir(dir);
 
}
 
void citire_director(const char *director, const char *output_dir)
{
 
    DIR *dir;
    if((dir = opendir(director)) == NULL)
    {
        perror("Eroare deschidere director");
        exit(1);
    }
    struct dirent *entry;
 
    char str[1024];
    while((entry=readdir(dir))!=NULL)
    {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
	        sprintf(str, "%s/%s", director, entry->d_name);
            process_file(str, output_dir);
        }
    } 
}
 
int main(int argc, char *argv[]) {
 
    if (argc != 3) {
        printf("Usage: %s <fisier_intrare>\n", argv[0]);
        return 1;
    }
 
    citire_director(argv[1], argv[2]);
    return 0;
}  