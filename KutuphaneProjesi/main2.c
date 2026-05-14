#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- VERI YAPILARI ---
typedef struct Book {
    char isbn[20];
    char title[100];
    int stock;
    struct Book* next;
} Book;

Book* head = NULL;

// --- DOSYA OKUMA VE YAZMA (KITAPLAR) ---
void loadBooksFromFile() {
    FILE* file = fopen("kutuphane_verisi.txt", "r");
    if (file == NULL) return;

    char isbn[20], title[100];
    int stock;
    head = NULL; // Listeyi sifirla
    
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
            while (current->next != NULL) {
                current = current->next;
            }
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

// --- KAYIT VE GIRIS (SADECE OGRENCI KAYDI) ---
void registerUser() {
    char username[50], password[50];
    printf("\n--- YENI OGRENCI KAYDI ---\n");
    printf("Kullanici Adi: "); scanf("%s", username);
    printf("Sifre: "); scanf("%s", password);

    // Artik rol sormuyoruz, direkt 'ogrenci' olarak yapistiriyoruz!
    FILE* file = fopen("kullanicilar.txt", "a");
    fprintf(file, "%s,%s,ogrenci\n", username, password);
    fclose(file);
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

// --- YONETICI: UYE ISLEMLERI (TEMP TAKTIGI) ---
void makeAdmin() {
    char targetUser[50];
    printf("Yonetici yapilacak kullanici adi: "); scanf("%s", targetUser);

    FILE *file = fopen("kullanicilar.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    char u[50], p[50], r[20];
    int found = 0;

    while(fscanf(file, "%[^,],%[^,],%s\n", u, p, r) != EOF) {
        if(strcmp(u, targetUser) == 0) {
            fprintf(temp, "%s,%s,yonetici\n", u, p); // Rolu degistirdik
            found = 1;
        } else {
            fprintf(temp, "%s,%s,%s\n", u, p, r); // Aynen kopyaladik
        }
    }
    fclose(file); fclose(temp);
    remove("kullanicilar.txt");
    rename("temp.txt", "kullanicilar.txt");

    if (found) printf("%s artik bir YONETICI!\n", targetUser);
    else printf("Kullanici bulunamadi!\n");
}

void deleteUser() {
    char targetUser[50];
    printf("Silinecek kullanici adi: "); scanf("%s", targetUser);

    FILE *file = fopen("kullanicilar.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    char u[50], p[50], r[20];
    int found = 0;

    while(fscanf(file, "%[^,],%[^,],%s\n", u, p, r) != EOF) {
        if(strcmp(u, targetUser) == 0) {
            found = 1; // Dosyaya yazmiyoruz, yani silmis oluyoruz!
        } else {
            fprintf(temp, "%s,%s,%s\n", u, p, r);
        }
    }
    fclose(file); fclose(temp);
    remove("kullanicilar.txt");
    rename("temp.txt", "kullanicilar.txt");

    if (found) printf("Kullanici sistemden silindi!\n");
    else printf("Kullanici bulunamadi!\n");
}

// --- YONETICI: KITAP ISLEMLERI ---
void addBook() {
    Book* newBook = (Book*)malloc(sizeof(Book));
    printf("ISBN: "); scanf("%s", newBook->isbn);
    printf("Kitap Adi: "); scanf("%s", newBook->title);
    printf("Stok: "); scanf("%d", &newBook->stock);
    
    newBook->next = head;
    head = newBook;
    saveBooksToFile();
    printf("Kitap basariyla eklendi!\n");
}

void deleteBook() {
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

    // Listeden koparip siliyoruz
    if (prev == NULL) head = current->next;
    else prev->next = current->next;

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
void borrowBook() {
    char searchTitle[100];
    printf("Odunc alinacak kitap: "); scanf("%s", searchTitle);
    
    Book* current = head;
    while (current != NULL) {
        if (strcmp(current->title, searchTitle) == 0) {
            if (current->stock > 0) {
                current->stock--;
                saveBooksToFile();
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

void returnBook() {
    char searchTitle[100];
    printf("Iade edilecek kitap: "); scanf("%s", searchTitle);
    
    Book* current = head;
    while (current != NULL) {
        if (strcmp(current->title, searchTitle) == 0) {
            current->stock++;
            saveBooksToFile();
            printf("Kitap iade edildi! Yeni stok: %d\n", current->stock);
            return;
        }
        current = current->next;
    }
    printf("Sistemde boyle bir kitap yok!\n");
}

// --- ANA MENU ---
int main() {
    loadBooksFromFile();
    int choice;
    char username[50], password[50], role[20];

    while(1) {
        printf("\n=== KUTUPHANE ANA MENU ===\n");
        printf("1. Giris Yap\n2. Kayit Ol\n0. Cikis\nSecim: ");
        scanf("%d", &choice);

        if (choice == 2) {
            registerUser(); // Artik sadece ogrenci kaydeder
        } 
        else if (choice == 1) {
            printf("Kullanici Adi: "); scanf("%s", username);
            printf("Sifre: "); scanf("%s", password);
            
            if (loginUser(username, password, role)) {
                printf("\nHOSGELDIN %s! (Yetki: %s)\n", username, role);
                
                int innerChoice;
                do {
                    // YONETICI MENUSU
                    if (strcmp(role, "yonetici") == 0) {
                        printf("\n[ YONETICI PANELI ]\n");
                        printf("1. Kitap Ekle\n2. Kitap Sil\n3. Kitaplari Listele\n");
                        printf("4. Yeni Yonetici Ata\n5. Kullanici Sil\n0. Cikis Yap\nSecim: ");
                        scanf("%d", &innerChoice);

                        if (innerChoice == 1) addBook();
                        else if (innerChoice == 2) deleteBook();
                        else if (innerChoice == 3) listBooks();
                        else if (innerChoice == 4) makeAdmin();
                        else if (innerChoice == 5) deleteUser();
                    } 
                    // OGRENCI MENUSU
                    else if (strcmp(role, "ogrenci") == 0) {
                        printf("\n[ OGRENCI PANELI ]\n");
                        printf("1. Kitaplari Listele\n2. Kitap Odunc Al\n3. Kitap Iade Et\n0. Cikis Yap\nSecim: ");
                        scanf("%d", &innerChoice);

                        if (innerChoice == 1) listBooks();
                        else if (innerChoice == 2) borrowBook();
                        else if (innerChoice == 3) returnBook();
                    }
                } while (innerChoice != 0);
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
