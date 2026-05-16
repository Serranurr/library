#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

// --- VERI YAPILARI ---
typedef struct Book {
    char isbn[20];
    char title[100];
    int stock;
    struct Book* next;
} Book;

Book* head = NULL;

// --- ISLEM GECMISI FONKSIYONLARI ---
void logTransaction(char* user, char* action, char* detail) {
    FILE* file = fopen("islem_gecmisi.txt", "a"); 
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
    printf("\n--- KUTUPHANE GENEL ISLEM GECMISI (LOGLAR) ---\n");
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    fclose(file);
}

void viewAllActiveLoans() {
    FILE* file = fopen("aktif_odunc.txt", "r");
    if (file == NULL) { 
        printf("\nSu an odunc verilmis hicbir kitap yok.\n"); 
        return; 
    }
    char u[50], b[100];
    int found = 0;
    printf("\n--- SU AN ODUNC VERILMIS TUM KITAPLAR ---\n");
    while (fscanf(file, "%[^,],%[^\n]\n", u, b) != EOF) {
        printf("Ogrenci: %s | Elindeki Kitap: %s\n", u, b);
        found = 1;
    }
    if (!found) printf("Su an odunc verilmis hicbir kitap yok.\n");
    fclose(file);
}

void viewUserHistory(char* username) {
    FILE* file = fopen("islem_gecmisi.txt", "r");
    if (file == NULL) { 
        printf("\nHenuz hicbir islem kaydiniz yok.\n"); 
        return; 
    }
    char line[256];
    char pattern[70];
    sprintf(pattern, " %s: ", username); 
    
    printf("\n--- KISISEL ISLEM GECMISINIZ ---\n");
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, pattern) != NULL) { 
            printf("%s", line);
            found = 1;
        }
    }
    if (!found) printf("Gecmis islem kaydiniz bulunamadi.\n");
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

        if (head == NULL) head = newBook;
        else {
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
    printf("Kullanici Adi (Iptal icin 0): "); scanf("%s", username);
    if (strcmp(username, "0") == 0) return; // Kullanici vazgecerse geri don
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
    printf("Yeni Kullanici Adi (Iptal icin 0): "); scanf("%s", u);
    if (strcmp(u, "0") == 0) return; // Vazgecis
    
    printf("Sifre: "); scanf("%s", p);
    printf("Rolu (yonetici / personel / ogrenci): "); scanf("%s", r);
    
    FILE* file = fopen("kullanicilar.txt", "a");
    fprintf(file, "%s,%s,%s\n", u, p, r);
    fclose(file);
    
    logTransaction(adminName, "YENI UYE EKLEDI", u);
    printf("Kullanici basariyla sisteme eklendi!\n");
}

void makeAdmin(char* adminName) {
    char targetUser[50];
    printf("Yonetici yapilacak kullanici adi (Iptal icin 0): "); scanf("%s", targetUser);
    if (strcmp(targetUser, "0") == 0) return;

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
    printf("Silinecek kullanici adi (Iptal icin 0): "); scanf("%s", targetUser);
    if (strcmp(targetUser, "0") == 0) return;

    FILE *file = fopen("kullanicilar.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    char u[50], p[50], r[20];
    int found = 0;

    while(fscanf(file, "%[^,],%[^,],%s\n", u, p, r) != EOF) {
        if(strcmp(u, targetUser) == 0) found = 1; 
        else fprintf(temp, "%s,%s,%s\n", u, p, r);
    }
    fclose(file); fclose(temp);
    remove("kullanicilar.txt");
    rename("temp.txt", "kullanicilar.txt");

    if (found) {
        printf("Kullanici sistemden silindi!\n");
        logTransaction(adminName, "KULLANICI SILDI", targetUser);
    } else printf("Kullanici bulunamadi!\n");
}

// --- ENVANTER YONETIMI ---
void addBook(char* activeUser) {
    Book* newBook = (Book*)malloc(sizeof(Book));
    printf("ISBN (Iptal icin 0): "); scanf("%s", newBook->isbn);
    
    // Iptal edilirse hafizada actigimiz yeri geri iade edip (free) cikiyoruz
    if (strcmp(newBook->isbn, "0") == 0) {
        free(newBook);
        return;
    }
    
    printf("Kitap Adi: "); scanf("%s", newBook->title);
    printf("Stok: "); scanf("%d", &newBook->stock);
    
    newBook->next = head;
    head = newBook;
    saveBooksToFile();
    
    logTransaction(activeUser, "KITAP EKLEDI", newBook->title);
    printf("Kitap basariyla eklendi!\n");
}

void deleteBook(char* activeUser) {
    char targetTitle[100];
    printf("Silinecek kitap adi (Iptal icin 0): "); scanf("%s", targetTitle);
    if (strcmp(targetTitle, "0") == 0) return;

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

    logTransaction(activeUser, "KITAP SILDI", current->title);
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

// --- OGRENCI: ODUNC, IADE VE AKTIF DOSYALAR ---
void viewMyBooks(char* username) {
    FILE* file = fopen("aktif_odunc.txt", "r");
    if (file == NULL) { 
        printf("\nUzerinizde hic kitap bulunmuyor.\n"); 
        return; 
    }
    char u[50], b[100];
    int found = 0;
    
    printf("\n--- UZERIMDEKI KITAPLAR ---\n");
    while (fscanf(file, "%[^,],%[^\n]\n", u, b) != EOF) {
        if (strcmp(u, username) == 0) {
            printf("- %s\n", b);
            found = 1;
        }
    }
    if (!found) printf("Uzerinizde hic kitap bulunmuyor.\n");
    fclose(file);
}

void borrowBook(char* username) {
    char searchTitle[100];
    printf("Odunc alinacak kitap (Iptal icin 0): "); scanf("%s", searchTitle);
    if (strcmp(searchTitle, "0") == 0) return; // Vazgecis
    
    Book* current = head;
    while (current != NULL) {
        if (strcmp(current->title, searchTitle) == 0) {
            if (current->stock > 0) {
                current->stock--;
                saveBooksToFile();
                
                FILE* f = fopen("aktif_odunc.txt", "a");
                fprintf(f, "%s,%s\n", username, searchTitle);
                fclose(f);

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
    printf("Iade edilecek kitap (Iptal icin 0): "); scanf("%s", searchTitle);
    if (strcmp(searchTitle, "0") == 0) return; // Vazgecis
    
    FILE* f = fopen("aktif_odunc.txt", "r");
    FILE* temp = fopen("temp_odunc.txt", "w");
    int removed = 0;
    
    if (f != NULL) {
        char u[50], b[100];
        while (fscanf(f, "%[^,],%[^\n]\n", u, b) != EOF) {
            if (!removed && strcmp(u, username) == 0 && strcmp(b, searchTitle) == 0) {
                removed = 1; 
            } else {
                fprintf(temp, "%s,%s\n", u, b);
            }
        }
        fclose(f); fclose(temp);
        remove("aktif_odunc.txt");
        rename("temp_odunc.txt", "aktif_odunc.txt");
    }

    if (!removed) {
        printf("Sistemde uzerinize kayitli boyle bir kitap yok!\n");
        return; 
    }

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
}

// --- ANA MOTOR (MENU) ---
int main() {
    loadBooksFromFile();
    int choice;
    char username[50], password[50], role[20];

    while(1) {
        printf("\n=== KUTUPHANE ANA MENU ===\n");
        printf("1. Giris Yap\n2. Kayit Ol\n0. Cikis\nSecim: ");
        scanf("%d", &choice);

        if (choice == 2) registerUser(); 
        else if (choice == 1) {
            printf("Kullanici Adi: "); scanf("%s", username);
            printf("Sifre: "); scanf("%s", password);
            
            if (loginUser(username, password, role)) {
                logTransaction(username, "SISTEME GIRDI", "-");
                printf("\nHOSGELDIN %s! (Yetki: %s)\n", username, role);
                
                int menuChoice;
                do {
                    // --- YONETICI MENUSU ---
                    if (strcmp(role, "yonetici") == 0) {
                        printf("\n[ YONETICI PANELI ]\n");
                        printf("1. Envanter Yonetimi (Kitap Islemleri)\n");
                        printf("2. Kullanici Yonetimi (Uye Islemleri)\n");
                        printf("3. Su An Kimde Ne Kitap Var? (Aktif Oduncler)\n"); 
                        printf("4. Genel Islem Gecmisini (Loglari) Goruntule\n");
                        printf("0. Cikis Yap\nSeciminiz: ");
                        scanf("%d", &menuChoice);

                        if (menuChoice == 1) {
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
                        else if (menuChoice == 2) {
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
                        else if (menuChoice == 3) viewAllActiveLoans(); 
                        else if (menuChoice == 4) viewHistory();
                    } 
                    // --- PERSONEL MENUSU ---
                    else if (strcmp(role, "personel") == 0) {
                        printf("\n[ PERSONEL PANELI ]\n");
                        printf("1. Envanter Yonetimi (Kitap Islemleri)\n");
                        printf("2. Su An Kimde Ne Kitap Var? (Aktif Oduncler)\n");
                        printf("3. Islem Gecmisini (Loglari) Goruntule\n");
                        printf("0. Cikis Yap\nSeciminiz: ");
                        scanf("%d", &menuChoice);

                        if (menuChoice == 1) {
                            int bookChoice;
                            do {
                                printf("\n--- ENVANTER YONETIMI ---\n");
                                printf("1. Kitap Ekle\n2. Kitap Sil\n3. Kitaplari Listele\n0. Personel Paneline Don\nSecim: ");
                                scanf("%d", &bookChoice);
                                if (bookChoice == 1) addBook(username);
                                else if (bookChoice == 2) deleteBook(username);
                                else if (bookChoice == 3) listBooks();
                            } while (bookChoice != 0);
                        }
                        else if (menuChoice == 2) viewAllActiveLoans(); 
                        else if (menuChoice == 3) viewHistory();
                    }
                    // --- OGRENCI MENUSU ---
                    else if (strcmp(role, "ogrenci") == 0) {
                        printf("\n[ OGRENCI PANELI ]\n");
                        printf("1. Kitaplari Listele\n2. Kitap Odunc Al\n3. Kitap Iade Et\n");
                        printf("4. Uzerimdeki Kitaplari Gor\n5. Gecmis Islem Kaydimi Gor\n0. Cikis Yap\nSecim: ");
                        scanf("%d", &menuChoice); 

                        if (menuChoice == 1) listBooks();
                        else if (menuChoice == 2) borrowBook(username);
                        else if (menuChoice == 3) returnBook(username);
                        else if (menuChoice == 4) viewMyBooks(username);
                        else if (menuChoice == 5) viewUserHistory(username);
                    }
                } while (menuChoice != 0);
                
                logTransaction(username, "SISTEMDEN CIKTI", "-");
            } else {
                printf("Hatali kullanici adi veya sifre!\n");
            }
        }
        else if (choice == 0) break;
    }
    return 0;
}
