#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define ROWS 8
#define COLS 10
#define MAX_MOVIES 4
#define MAX_TICKETS 4
#define BASE_PRICE 800
#define PEAK_PRICE 1200
#define PEAK_LIMIT 70
#define KEY "CINESYNC"
#define USERS_FILE "users.txt"
#define BOOKINGS_FILE "bookings.txt"


struct Movie {
    char title[40];
    char time[10];
    int screen;
};
struct User {
    char username[30];
    char password[128];
    char cnic[14];
};
struct Booking {
    char cnic[14];
    int movie;
    int row;
    int col;
    float price;
    char token[20];
};


struct Movie movies[MAX_MOVIES] = {
    {"Dune: Part Two", "14:00", 1},
    {"Inside Out 2", "16:30", 2},
    {"Oppenheimer", "19:00", 3},
    {"Interstellar", "21:30", 1}
};

void printHeader(char title[])
{
    int len = strlen(title);
    int total_Padding = 50 - len;
    int left_P = total_Padding / 2;
    int right_P = total_Padding - left_P;

    printf("\n+--------------------------------------------------+\n");
    printf("|%*s%s%*s|\n", left_P, "", title, right_P, "");
    printf("+--------------------------------------------------+\n");
}

void readLine(char text[], int size)
{
    if(fgets(text, size, stdin) == NULL){
        printf("\nInput ended. Program closing.\n");
        exit(0);
    }
    if(strchr(text, '\n') == NULL){
        int ch;
        while((ch = getchar()) != '\n' && ch != EOF){ }
    } else {
        text[strcspn(text, "\n")] = '\0';
    }
}

int readInt(char message[], int min, int max)
{
    char line[40], extra;
    int number;
    while(1){ //means while true
        printf("%s", message);
        readLine(line, sizeof(line));

        if((sscanf(line, "%d%c", &number, &extra) == 1) && (number >= min) && (number <= max)){
            return number;
        }
        printf("Enter a whole number from %d to %d.\n", min, max);
    }
}

int isDigitsOnly(char text[])
{
    if(text[0] == '\0'){
        return 0;
    }
    for(int i = 0; text[i] != '\0'; i++){
        if(!isdigit(text[i])){
            return 0;
        }
    }
    return 1;
}


void xorCrypt(char text[], int length)
{
    int keyLength = strlen(KEY);
    for(int i = 0; i < length; i++){
        text[i] = text[i] ^ KEY[i % keyLength];
    }
}

void toHex(char text[], int length, char hex[])
{
    for(int i = 0; i < length; i++)
        sprintf(hex + i * 2, "%02X", text[i]);

    hex[length * 2] = '\0';
}


int registerUser(void){
    struct User newUser, user;
    char password[30], scrambled[30], hex[128];
    FILE *fp;

    printHeader("USER REGISTRATION");
    printf("Username (no spaces): ");
    readLine(newUser.username, sizeof(newUser.username));

    if(newUser.username[0] == '\0' || strchr(newUser.username, ' ')){
        printf("Invalid username.\n");
        return 0;
    }
    printf("Password (at least 4 characters, no spaces): ");
    readLine(password, sizeof(password));

    if(strlen(password) < 4 || strchr(password, ' ')){
        printf("Invalid password.\n");
        return 0;
    }
    printf("CNIC (13 digits, without dashes): ");
    readLine(newUser.cnic, sizeof(newUser.cnic));

    if(strlen(newUser.cnic) != 13 || !isDigitsOnly(newUser.cnic)){
        printf("CNIC must contain exactly 13 digits.\n");
        return 0;
    }

    fp =fopen(USERS_FILE, "r");
    if(fp != NULL){
        while(fscanf(fp, "%29s %127s %13s",user.username, user.password, user.cnic) == 3){
           
            if((strcmp(user.username, newUser.username) == 0) || (strcmp(user.cnic, newUser.cnic) == 0)){
                printf("Username or CNIC already registered.\n");
                fclose(fp);
                return 0;
            }
        }
        fclose(fp);
    }

    strcpy(scrambled, password);
    xorCrypt(scrambled, strlen(scrambled));
    toHex(scrambled, strlen(password), hex);

    fp =fopen(USERS_FILE, "a");

    if(fp == NULL){
        printf("Could not open users file.\n");
        return 0;
    }

    fprintf(fp, "%s %s %s\n", newUser.username, hex, newUser.cnic);
    fclose(fp);

    printf("Registration successful. You can now log in.\n");
    return 1;
}

int loginUser(char username[], char cnic[]){
    struct User user;
    char enteredName[30], password[30], scrambled[30], hex[128];
    // FILE *fp;

    printHeader("USER LOGIN");
    printf("Username: ");
    readLine(enteredName, sizeof(enteredName));
    printf("Password: ");
    readLine(password, sizeof(password));
    strcpy(scrambled, password);
    xorCrypt(scrambled, strlen(scrambled));
    toHex(scrambled, strlen(password), hex);

    FILE *fp =fopen(USERS_FILE, "r");
    if(fp == NULL){
        printf("No registered users found.\n");
        return 0;
    }

    while(fscanf(fp, "%29s %127s %13s",user.username, user.password, user.cnic) == 3){
        if(strcmp(user.username, enteredName) == 0 && strcmp(user.password, hex) == 0){
            
            
            strcpy(username, user.username);
            strcpy(cnic, user.cnic);
            fclose(fp);
            char welcomeMsg[60];
            sprintf(welcomeMsg, "Welcome, %s!", username);
            printHeader(welcomeMsg);
            return 1;
        }
    }

    fclose(fp);
    printf("Incorrect username or password.\n");
    return 0;
}


void showMovies(void){
    printHeader("NOW SHOWING");
    printf("+-----+------------------------+----------+--------+\n");
    printf("| ID  | Movie                  | Time     | Screen |\n");
    printf("+-----+------------------------+----------+--------+\n");

    for(int i=0;i<MAX_MOVIES;i++)
        printf("| %-3d | %-22s | %-8s | %-6d |\n",i + 1, movies[i].title, movies[i].time, movies[i].screen);

    printf("+-----+------------------------+----------+--------+\n");
}

void initSeats(char seats[ROWS][COLS]){
    for(int i=0;i<ROWS;i++){
        for(int j=0;j<COLS;j++)
            seats[i][j]='O';
    }
}

void loadSeats(int movieId, char seats[ROWS][COLS]){
    struct Booking booking;
    FILE *fp;
    initSeats(seats);
    fp =fopen(BOOKINGS_FILE, "r");

    if(fp == NULL){
        return;
    }
    while(fscanf(fp, "%13s %d %d %d %f %19s", booking.cnic, &booking.movie, &booking.row, &booking.col, &booking.price, booking.token) == 6){
        
        
        if(booking.movie == movieId && booking.row >= 1 && booking.row <= ROWS && booking.col >= 1 && booking.col <= COLS){
            
            seats[booking.row-1][booking.col-1] = 'X';
        }

    }

    fclose(fp);
}

void displaySeats(char seats[ROWS][COLS]){
    printf("\n            =========== SCREEN ===========\n");
    printf("        ");

    for(int i=1; i<=COLS; i++)
        printf("%2d  ", i);

    printf("\n");

    for(int i=0;i<ROWS;i++){
        printf("  R%-2d   ", i + 1);
        for(int j=0;j<COLS;j++){
            printf("[%c] ", seats[i][j]);

        }


        printf("\n");
    }

    printf("\n        [O] Available        [X] Booked\n");
}

int countFreeSeats(char seats[ROWS][COLS]){
    int freeSeats = 0;
    for(int i=0;i<ROWS;i++){
        for(int j=0; j<COLS; j++){
            if(seats[i][j] == 'O'){
                freeSeats++;
            }
        }
    }
    return freeSeats;
}
int countPurchased(char cnic[], int movieId){
    struct Booking booking;
    int count = 0;
    FILE *fp =fopen(BOOKINGS_FILE, "r");

    if(fp == NULL)
        return 0;

    while(fscanf(fp, "%13s %d %d %d %f %19s", booking.cnic, &booking.movie, &booking.row, &booking.col, &booking.price, booking.token) == 6){
        if(strcmp(booking.cnic, cnic) == 0 && booking.movie == movieId)
            count++;
    }

    fclose(fp);
    return count;
}

float getOccupancy(char seats[ROWS][COLS]){
    int booked = ROWS * COLS - countFreeSeats(seats);
    return (float)booked * 100.0f / (ROWS * COLS);
}

float getPrice(float occupancy){
    if(occupancy >= PEAK_LIMIT)
        return PEAK_PRICE;

    return BASE_PRICE;
}

void makeToken(int movie, int row, int col, char token[]){
    sprintf(token, "T%d%02d%02d%04d", movie, row, col, rand() % 10000);
}


int saveBooking(struct Booking booking){
    FILE *fp =fopen(BOOKINGS_FILE, "a");

    if(fp == NULL){
        printf("Could not save booking.\n");
        return 0;
    }

    fprintf(fp, "%s %d %d %d %.2f %s\n", booking.cnic, booking.movie, booking.row, booking.col, booking.price, booking.token);

    if(fclose(fp) != 0){
        printf("Error closing bookings file.\n");
        return 0;
    }

    return 1;
}

int writeTicket(struct Booking booking){
    char plain[128], hex[256], filename[40];
    FILE *fp;

    sprintf(plain, "%s|%d|%d|%d|%.2f|%s", booking.cnic, booking.movie, booking.row, booking.col, booking.price, booking.token);

    int length = strlen(plain);
    xorCrypt(plain, length);
    toHex(plain, length, hex);

    sprintf(filename, "ticket_%s.txt", booking.token);
    fp =fopen(filename,"w");

    if(fp == NULL){
        printf("Could not create ticket file.\n");
        return 0;
    }

    fprintf(fp,"%s\n",hex);
    fclose(fp);
    return 1;
}

void bookTicket(char cnic[]){
    char seats[ROWS][COLS];
    struct Booking booking;
    int movie, quantity, allowed, freeSeats;

    showMovies();
    movie =readInt("Select movie (0 to cancel): ", 0, MAX_MOVIES);

    if(movie==0){
        return;
    }

    loadSeats(movie, seats);

    allowed =MAX_TICKETS -countPurchased(cnic, movie);
    freeSeats =countFreeSeats(seats);

    if(allowed <=0){
        printf("Booking blocked: you already have the maximum %d tickets.\n",
               MAX_TICKETS);
        return;
    }

    if(freeSeats ==0){
        printf("This movie is sold out.\n");
        return;
    }

    displaySeats(seats);

    printf("Current occupancy: %.1f%%\n", getOccupancy(seats));
    printf("Current ticket price: Rs. %.2f\n", getPrice(getOccupancy(seats)));

    if(allowed>freeSeats){
        allowed=freeSeats;
    }

    quantity = readInt("Number of tickets: ", 1, allowed);

    strcpy(booking.cnic, cnic);
    booking.movie = movie;

    for(int i=1;i<=quantity;i++){
        printf("\n--- Ticket %d of %d ---\n", i, quantity);

        while(1){
            booking.row= readInt("Row: ", 1, ROWS);
            booking.col= readInt("Seat number: ", 1, COLS);

            if(seats[booking.row - 1][booking.col - 1] == 'X')
                printf("Seat already booked. Choose another.\n");
            else
                break;
        }

        booking.price = getPrice(getOccupancy(seats));
        printf("Price: Rs. %.2f. Confirm? (1=Yes, 0=No): ",
               booking.price);

        char answer[8];
        readLine(answer, sizeof(answer));

        if(strcmp(answer, "1") != 0){
            printf("Ticket cancelled.\n");
            continue;
        }
        makeToken(movie, booking.row, booking.col, booking.token);
        if(!writeTicket(booking))
            continue;

        if(!saveBooking(booking)){
            char filename[40];
            sprintf(filename, "ticket_%s.txt", booking.token);
            remove(filename);
            continue;
        }
        seats[booking.row-1][booking.col-1] = 'X';

        printf("\n+--------------------------------------------------+\n");
        printf("|                BOOKING SUCCESSFUL                |\n");
        printf("+--------------------------------------------------+\n");
        printf("| Movie       : %-34s |\n", movies[movie - 1].title);
        printf("| Time        : %-34s |\n", movies[movie - 1].time);
        printf("| Screen      : %-34d |\n", movies[movie - 1].screen);
        printf("| Seat        : Row %-2d, Seat %-21d |\n",
               booking.row, booking.col);
        printf("| Price       : Rs. %-30.2f |\n", booking.price);
        printf("| Ticket Token: %-34s |\n", booking.token);
        printf("+--------------------------------------------------+\n");

        if(getOccupancy(seats) >= PEAK_LIMIT)
            printf("Remaining seats will cost Rs. %.2f.\n", PEAK_PRICE);
    }
}

void myBookings(char cnic[]){
    struct Booking booking;
    int found = 0;
    FILE *fp =fopen(BOOKINGS_FILE, "r");

    printHeader("MY BOOKINGS");
    if(fp == NULL){
        printf("No bookings found.\n");
        return;
    }
    printf("+----------------------+----------+--------+--------+----------+\n");
    printf("| Movie                | Time     | Screen | Seat   | Price    |\n");
    printf("+----------------------+----------+--------+--------+----------+\n");

    while(fscanf(fp, "%13s %d %d %d %f %19s", booking.cnic, &booking.movie, &booking.row, &booking.col, &booking.price, booking.token) == 6){

        if((strcmp(booking.cnic, cnic) == 0) &&(booking.movie >= 1) && (booking.movie <= MAX_MOVIES)){
            
            char seat[12];
            
            sprintf(seat, "R%d C%d", booking.row, booking.col);
            printf("| %-20s | %-8s | %-6d | %-6s | %8.2f |\n", movies[booking.movie - 1].title, movies[booking.movie - 1].time, movies[booking.movie - 1].screen, seat, booking.price);

            printf("| Token: %-53s |\n", booking.token);
            found = 1;
        }
    }

    printf("+----------------------+----------+--------+--------+----------+\n");
    fclose(fp);

    if(!found)
        printf("You have no bookings.\n");
}

void verifyTicket(void){
    struct Booking booking;
    char token[20], filename[40], hex[256], exp[128], expHex[256];
    FILE *fp;
    int length;
    printHeader("VERIFY TICKET");
    printf("Enter ticket token: ");
    readLine(token, sizeof(token));

    if(token[0] == '\0'){
        printf("Token cannot be empty.\n");
        return;
    }
    for(int i = 0; token[i] != '\0'; i++){
        if(!isalnum(token[i])){
            printf("Invalid token format.\n");
            return;
        }
    }
    sprintf(filename, "ticket_%s.txt", token);
    fp =fopen(filename, "r");

    if(fp == NULL){
        printf("INVALID: Ticket file not found.\n");
        return;
    }
    if(fscanf(fp, "%255s", hex) != 1){
        fclose(fp);
        printf("INVALID: Ticket file is empty.\n");
        return;
    }
    fclose(fp);


    fp =fopen(BOOKINGS_FILE, "r");

    if(fp == NULL){
        printf("INVALID: No booking records found.\n");
        return;
    }

    while(fscanf(fp, "%13s %d %d %d %f %19s", booking.cnic, &booking.movie, &booking.row, &booking.col, &booking.price, booking.token) == 6){
        
        if(strcmp(booking.token, token) == 0){
            fclose(fp);

            if(booking.movie < 1 || booking.movie > MAX_MOVIES || booking.row < 1 || booking.row > ROWS || booking.col < 1 || booking.col > COLS){
                printf("INVALID: Booking record is incorrect.\n");
                return;
            }

            sprintf(exp, "%s|%d|%d|%d|%.2f|%s",booking.cnic, booking.movie, booking.row, booking.col, booking.price, booking.token);

            length = strlen(exp);
            xorCrypt(exp, length);
            toHex(exp, length, expHex);

            if(strcmp(expHex, hex) == 0){
                printf("VALID TICKET\n");
                printf("Movie: %s (%s)\n",movies[booking.movie - 1].title, movies[booking.movie - 1].time);
                printf("Seat: Row %d, Seat %d\n",
                       booking.row, booking.col);
            } else {
                printf("INVALID: Ticket does not match booking records.\n");
            }
            return;
        }
    }

    fclose(fp);
    printf("INVALID: Token not found in booking records.\n");
}

void customerMenu(char username[], char cnic[]){
    int choice;
    do {
        char welcomeStr[40];
        sprintf(welcomeStr, "Welcome %s", username);
        int wLen = strlen(welcomeStr);
        int wPad = 50 - wLen;
        int wLeft = wPad / 2;
        int wRight = wPad - wLeft;

        printf("\n+--------------------------------------------------+\n");
        printf("|                 CINESYNC CINEMAS                 |\n");
        printf("|%*s%s%*s|\n", wLeft, "", welcomeStr, wRight, "");
        printf("+--------------------------------------------------+\n");
        printf("|  1.  View Movies                                 |\n");
        printf("|  2.  Book Tickets                                |\n");
        printf("|  3.  My Bookings                                 |\n");
        printf("|  4.  Verify Ticket                               |\n");
        printf("|  5.  Logout                                      |\n");
        printf("+--------------------------------------------------+\n");

        choice = readInt("Choice: ", 1, 5);
        switch (choice){
            case 1:
                showMovies();
                break;
            case 2: 
                bookTicket(cnic); 
                break;
            case 3:
                myBookings(cnic); 
                break;
            case 4:
                verifyTicket(); 
                break;
            case 5:
                printf("Logged out.\n"); 
                break;
             
        }
    } while(choice != 5);
}

int main(){
    int choice;
    char username[30], cnic[14];
    srand((unsigned int)time(NULL));
    do {
        printf("\n+--------------------------------------------------+\n");
        printf("|                                                  |\n");
        printf("|                  CINESYNC CINEMAS                |\n");
        printf("|                                                  |\n");
        printf("+--------------------------------------------------+\n");
        printf("|  1.  Register                                    |\n");
        printf("|  2.  Login                                       |\n");
        printf("|  3.  View Movies                                 |\n");
        printf("|  4.  Verify Ticket                               |\n");
        printf("|  5.  Exit                                        |\n");
        printf("+--------------------------------------------------+\n");

        choice = readInt("Choice: ", 1, 5);

        switch (choice){
            case 1:
                registerUser();
                break;
            case 2:
                if(loginUser(username, cnic))
                    customerMenu(username, cnic);
                break;
            case 3:
                showMovies();
                break;
            case 4:
                verifyTicket();
                break;
            case 5:
                printf("Thank you for using CineSync!. Goodbye!\n");
                break;
        }
    }while(choice != 5);

    return 0;
}