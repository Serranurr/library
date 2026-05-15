#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // Loglar (gecmis) icin gerekli

// --- VERI YAPILARI ---
typedef struct Book {
    char isbn[20];
    char title[100];
    int stock;
    struct Book* next;
} Book;

Book* head = NULL;

// --- ISLEM GECMISI (LOG) FONKSIYONLARI ---
void logTransaction(char* user, char* action, char* detail) {
    FILE* file = fopen("islem_gecmisi.txt", "a"); // Dosya yoksa kendi olusturur
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    fprintf(file, "[%02d.%02d.%d %02d:%02d] %s: %s - %s\n", 
            tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, 
            tm.tm_hour, tm.tm_min, user, action, detail);
    fclose(file);
}

void viewHistory() {
    FILE* file = fopen("islem_gecmisi.txt", "r");
    if (file == NULL) { 
        printf("\nHenuz hicbir islem yapilmamis.\n"); 
        return; 
    }
    char line[256];
    printf("\n--- KUTUPHANE ISLEM GECMISI ---\n");
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    fclose(file);
}

// --- DOSYA OKUMA VE YAZMA (KITAPLAR) ---
void loadBooksFromFile() {
    FILE* file = fopen("kutuphane_verisi.txt", "r");
    if (file == NULL) return;

    char isbn[20], title[100];
    int stock;
    head = NULL; 
    
    while (fscanf(file, "%[^,],%[^,],%d\n", isbn, title, &stock) != EOF) {
        Book* newBook = (Book*)malloc(sizeof(Book));
        strcpy(newBook->isbn, isbn);
        strcpy(newBook->title, title);
        newBook->stock = stock;
        newBook->next = NULL;

        if (head == NULL) {
            head = newBook;
        } else {
            Book* current = head;
            while (current->next != NULL) current = current->next;
            current->next = newBook;
        }
    }
    fclose(file);
}

void saveBooksToFile() {
    FILE* file = fopen("kutuphane_verisi.txt", "w");
    Book* current = head;
    while (current != NULL) {
        fprintf(file, "%s,%s,%d\n", current->isbn, current->title, current->stock);
        current = current->next;
    }
    fclose(file);
}

// --- KAYIT VE GIRIS (SISTEM GIRISI) ---
void registerUser() {
    char username[50], password[50];
    printf("\n--- YENI OGRENCI KAYDI ---\n");
    printf("Kullanici Adi: "); scanf("%s", username);
    printf("Sifre: "); scanf("%s", password);

    FILE* file = fopen("kullanicilar.txt", "a");
    fprintf(file, "%s,%s,ogrenci\n", username, password);
    fclose(file);
    
    logTransaction(username, "SISTEME KAYIT OLDU", "-");
    printf("Kayit basarili! Ogrenci olarak giris yapabilirsiniz.\n\n");
}

int loginUser(char* inputUser, char* inputPass, char* loggedRole) {
    FILE* file = fopen("kullanicilar.txt", "r");
    if (file == NULL) return 0;

    char fileUser[50], filePass[50], fileRole[20];
    while (fscanf(file, "%[^,],%[^,],%s\n", fileUser, filePass, fileRole) != EOF) {
        if (strcmp(inputUser, fileUser) == 0 && strcmp(inputPass, filePass) == 0) {
            strcpy(loggedRole, fileRole);
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

// --- YONETICI: KULLANICI YONETIMI ---
void addUserByAdmin(char* adminName) {
    char u[50], p[50], r[20];
    printf("Yeni Kullanici Adi: "); scanf("%s", u);
    printf("Sifre: "); scanf("%s", p);
    printf("Rolu (yonetici / ogrenci): "); scanf("%s", r);
    
    FILE* file = fopen("kullanicilar.txt", "a");
    fprintf(file, "%s,%s,%s\n", u, p, r);
    fclose(file);
    
    logTransaction(adminName, "YENI UYE EKLEDI", u);
    printf("Kullanici basariyla sisteme eklendi!\n");
}

void makeAdmin(char* adminName) {
    char targetUser[50];
    printf("Yonetici yapilacak kullanici adi: "); scanf("%s", targetUser);

    FILE *file = fopen("kullanicilar.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    char u[50], p[50], r[20];
    int found = 0;

    while(fscanf(file, "%[^,],%[^,],%s\n", u, p, r) != EOF) {
        if(strcmp(u, targetUser) == 0) {
            fprintf(temp, "%s,%s,yonetici\n", u, p); 
            found = 1;
        } else {
            fprintf(temp, "%s,%s,%s\n", u, p, r); 
        }
    }
    fclose(file); fclose(temp);
    remove("kullanicilar.txt");
    rename("temp.txt", "kullanicilar.txt");

    if (found) {
        printf("%s artik bir YONETICI!\n", targetUser);
        logTransaction(adminName, "YONETICI YAPTI", targetUser);
    } else printf("Kullanici bulunamadi!\n");
}

void deleteUser(char* adminName) {
    char targetUser[50];
    printf("Silinecek kullanici adi: "); scanf("%s", targetUser);

    FILE *file = fopen("kullanicilar.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    char u[50], p[50], r[20];
    int found = 0;

    while(fscanf(file, "%[^,],%[^,],%s\n", u, p, r) != EOF) {
        if(strcmp(u, targetUser) == 0) {
            found = 1; 
        } else {
            fprintf(temp, "%s,%s,%s\n", u, p, r);
        }
    }
    fclose(file); fclose(temp);
    remove("kullanicilar.txt");
    rename("temp.txt", "kullanicilar.txt");

    if (found) {
        printf("Kullanici sistemden silindi!\n");
        logTransaction(adminName, "KULLANICI SILDI", targetUser);
    } else printf("Kullanici bulunamadi!\n");
}

// --- YONETICI: ENVANTER YONETIMI ---
void addBook(char* adminName) {
    Book* newBook = (Book*)malloc(sizeof(Book));
    printf("ISBN: "); scanf("%s", newBook->isbn);
    printf("Kitap Adi: "); scanf("%s", newBook->title);
    printf("Stok: "); scanf("%d", &newBook->stock);
    
    newBook->next = head;
    head = newBook;
    saveBooksToFile();
    
    logTransaction(adminName, "KITAP EKLEDI", newBook->title);
    printf("Kitap basariyla eklendi!\n");
}

void deleteBook(char* adminName) {
    char targetTitle[100];
    printf("Silinecek kitap adi: "); scanf("%s", targetTitle);

    Book* current = head;
    Book* prev = NULL;

    while (current != NULL && strcmp(current->title, targetTitle) != 0) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("Kitap bulunamadi!\n");
        return;
    }

    if (prev == NULL) head = current->next;
    else prev->next = current->next;

    logTransaction(adminName, "KITAP SILDI", current->title);
    free(current);
    saveBooksToFile();
    printf("Kitap sistemden tamamen silindi!\n");
}

void listBooks() {
    printf("\n--- KUTUPHANE ENVANTERI ---\n");
    Book* current = head;
    if (current == NULL) printf("Kutuphane bos.\n");
    while (current != NULL) {
        printf("Kitap: %s | Stok: %d\n", current->title, current->stock);
        current = current->next;
    }
}

// --- OGRENCI: ODUNC VE IADE ---
void borrowBook(char* username) {
    char searchTitle[100];
    printf("Odunc alinacak kitap: "); scanf("%s", searchTitle);
    
    Book* current = head;
    while (current != NULL) {
        if (strcmp(current->title, searchTitle) == 0) {
            if (current->stock > 0) {
                current->stock--;
                saveBooksToFile();
                logTransaction(username, "ODUNC ALDI", searchTitle);
                printf("Kitabi aldiniz! Kalan stok: %d\n", current->stock);
            } else {
                printf("Bu kitabin stoku bitmis!\n");
            }
            return;
        }
        current = current->next;
    }
    printf("Kitap bulunamadi!\n");
}

void returnBook(char* username) {
    char searchTitle[100];
    printf("Iade edilecek kitap: "); scanf("%s", searchTitle);
    
    Book* current = head;
    while (current != NULL) {
        if (strcmp(current->title, searchTitle) == 0) {
            current->stock++;
            saveBooksToFile();
            logTransaction(username, "IADE ETTI", searchTitle);
            printf("Kitap iade edildi! Yeni stok: %d\n", current->stock);
            return;
        }
        current = current->next;
    }
    printf("Sistemde boyle bir kitap yok!\n");
}

// --- ANA MOTOR (MENU HIYERARSISI) ---
int main() {
    loadBooksFromFile();
    int choice;
    char username[50], password[50], role[20];

    while(1) {
        printf("\n=== KUTUPHANE ANA MENU ===\n");
        printf("1. Giris Yap\n2. Kayit Ol\n0. Cikis\nSecim: ");
        scanf("%d", &choice);

        if (choice == 2) {
            registerUser(); 
        } 
        else if (choice == 1) {
            printf("Kullanici Adi: "); scanf("%s", username);
            printf("Sifre: "); scanf("%s", password);
            
            if (loginUser(username, password, role)) {
                logTransaction(username, "SISTEME GIRDI", "-");
                printf("\nHOSGELDIN %s! (Yetki: %s)\n", username, role);
                
                int adminChoice;
                do {
                    // --- YONETICI MENUSU (KATMANLI) ---
                    if (strcmp(role, "yonetici") == 0) {
                        printf("\n[ YONETICI PANELI ]\n");
                        printf("1. Envanter Yonetimi (Kitap Islemleri)\n");
                        printf("2. Kullanici Yonetimi (Uye Islemleri)\n");
                        printf("3. Islem Gecmisini (Loglari) Goruntule\n");
                        printf("0. Cikis Yap\nSeciminiz: ");
                        scanf("%d", &adminChoice);

                        // 1. ALT MENU: ENVANTER YONETIMI
                        if (adminChoice == 1) {
                            int bookChoice;
                            do {
                                printf("\n--- ENVANTER YONETIMI ---\n");
                                printf("1. Kitap Ekle\n2. Kitap Sil\n3. Kitaplari Listele\n0. Yonetici Paneline Don\nSecim: ");
                                scanf("%d", &bookChoice);

                                if (bookChoice == 1) addBook(username);
                                else if (bookChoice == 2) deleteBook(username);
                                else if (bookChoice == 3) listBooks();
                            } while (bookChoice != 0);
                        }
                        // 2. ALT MENU: KULLANICI YONETIMI
                        else if (adminChoice == 2) {
                            int userChoice;
                            do {
                                printf("\n--- KULLANICI YONETIMI ---\n");
                                printf("1. Yeni Uye Ekle\n2. Uye Sil\n3. Yonetici Yap\n0. Yonetici Paneline Don\nSecim: ");
                                scanf("%d", &userChoice);

                                if (userChoice == 1) addUserByAdmin(username);
                                else if (userChoice == 2) deleteUser(username);
                                else if (userChoice == 3) makeAdmin(username);
                            } while (userChoice != 0);
                        }
                        // 3. ALT MENU: GECMIS
                        else if (adminChoice == 3) {
                            viewHistory();
                        }
                    } 
                    // --- OGRENCI MENUSU (SADE) ---
                    else if (strcmp(role, "ogrenci") == 0) {
                        printf("\n[ OGRENCI PANELI ]\n");
                        printf("1. Kitaplari Listele\n2. Kitap Odunc Al\n3. Kitap Iade Et\n0. Cikis Yap\nSecim: ");
                        scanf("%d", &adminChoice); 

                        if (adminChoice == 1) listBooks();
                        else if (adminChoice == 2) borrowBook(username);
                        else if (adminChoice == 3) returnBook(username);
                    }
                } while (adminChoice != 0);
                
                logTransaction(username, "SISTEMDEN CIKTI", "-");
            } else {
                printf("Hatali kullanici adi veya sifre!\n");
            }
        }
        else if (choice == 0) {
            break;
        }
    }
    return 0;
}
