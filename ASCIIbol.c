#define _WIN32_WINNT 0x0500 //for screen resize
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "windows.h"

int player_Faint[2], current_Mon[2]; //[0] = Player 1, //[1] = Player 2. player loses when player_Faint becomes 6
int players_Mons[12], Mon_HPleft[12], Mon_HP[12], Mon_ATK[12], Mon_DEF[12], Mon_SPA[12], Mon_SPD[12], Mon_SPE[12], Mon_Type1[12], Mon_Type2[12];
char Mon_Names[12][11], Mon_Ascii[12][4][13]; //Mon_Names=10 characters max; Mon_Ascii=12 characters per line, 4 lines
int Mon_SpriteColors[12][4];
int knockout_Flag; //0 or 1 = correspondent player mon got trashed out, 2 = regular turn
int type_Chart[18][18];
HANDLE color_Text;

void printAtk(int atk) //literally assigns int type to print an attack name. SetConsoleTextAttribute(color_Text, 7) not part of this for special purposes
{
    switch (atk)
    {
        case 0:
        SetConsoleTextAttribute(color_Text, 15);
        printf("Normal");
        break;
        case 1:
        SetConsoleTextAttribute(color_Text, 4);
        printf("Fighting");
        break;
        case 2:
        SetConsoleTextAttribute(color_Text, 15);
        printf("Flying");
        break;
        case 3:
        SetConsoleTextAttribute(color_Text, 5);
        printf("Poison");
        break;
        case 4:
        SetConsoleTextAttribute(color_Text, 6);
        printf("Ground");
        break;
        case 5:
        SetConsoleTextAttribute(color_Text, 6);
        printf("Rock");
        break;
        case 6:
        SetConsoleTextAttribute(color_Text, 2);
        printf("Bug");
        break;
        case 7:
        SetConsoleTextAttribute(color_Text, 1);
        printf("Ghost");
        break;
        case 8:
        SetConsoleTextAttribute(color_Text, 8);
        printf("Steel");
        break;
        case 9:
        SetConsoleTextAttribute(color_Text, 12);
        printf("Fire");
        break;
        case 10:
        SetConsoleTextAttribute(color_Text, 9);
        printf("Water");
        break;
        case 11:
        SetConsoleTextAttribute(color_Text, 10);
        printf("Grass");
        break;
        case 12:
        SetConsoleTextAttribute(color_Text, 14);
        printf("Electric");
        break;
        case 13:
        SetConsoleTextAttribute(color_Text, 5);
        printf("Psychic");
        break;
        case 14:
        SetConsoleTextAttribute(color_Text, 11);
        printf("Ice");
        break;
        case 15:
        SetConsoleTextAttribute(color_Text, 1);
        printf("Dragon");
        break;
        case 16:
        SetConsoleTextAttribute(color_Text, 15); //
        printf("Dark");
        break;
        case 17:
        SetConsoleTextAttribute(color_Text, 13);
        printf("Fairy");
        break;
        default:
        printf("ERROR");
    }
}

void printAtkThenColorless(int type) //does printAtk then removes the text's color
{
    printAtk(type);
    SetConsoleTextAttribute(color_Text, 7);
}

void printHPBar(int pointer) //prints the colored HP bar
{
    if (Mon_HPleft[pointer] != 0)
    {
        if (Mon_HPleft[pointer] > Mon_HP[pointer] / 2)
            SetConsoleTextAttribute(color_Text, 34); //GREEN BAR
        else if (Mon_HPleft[pointer] > Mon_HP[pointer] / 10)
            SetConsoleTextAttribute(color_Text, 102); //"ORANGE" BAR
        else
            SetConsoleTextAttribute(color_Text, 68); //RED BAR

        int HPPerc = 12 * Mon_HPleft[pointer]/Mon_HP[pointer];

        for (int HPBar = 0; HPBar <= HPPerc; HPBar++)
            printf(" ");
        SetConsoleTextAttribute(color_Text, 7); //COLORLESS
    }
    if (pointer < 6 || Mon_HPleft[pointer] != Mon_HP[pointer]) //prevents double line skip
        printf("\n");
}

void printHPVal(int HP) //example prints "100" or "  5"
{
    if (HP < 100)
    {
        printf(" ");
        if (HP < 10)
            printf(" ");
    }
    printf("%d", HP);
}

void printHPNumber(int pointer) //example prints "HP 100/100"
{
    //printf("HP %d / %d\n", Mon_HPleft[pointer], Mon_HP[pointer]);
    printf("HP ");
    printHPVal(Mon_HPleft[pointer]);
    printf(" / ");
    printHPVal(Mon_HP[pointer]);
}

void printWhiteBall(int counter)
{
    for (int i = 0; i < 6 - counter; i++)
        printf(" O");
}

void printRedBall(int redCounter)
{
    SetConsoleTextAttribute(color_Text, 4);
    printWhiteBall(redCounter);
    SetConsoleTextAttribute(color_Text, 7);
}

void printASCII(int pointer)
{
    for (int temp = 0; temp < 4; temp++)
    {
        if (pointer >= 6) //>=6 = player 2
            printf("%*s", 24, "");
        SetConsoleTextAttribute(color_Text, Mon_SpriteColors[pointer][temp]);
        printf("%s", Mon_Ascii[pointer][temp]);
        /*'\s' is its own character, so the console cursor could make an additional line break for player 2's sprites
        This workaround forces a line break for player 1 instead*/
        if (pointer < 6)
        {
            int lenghtString = strlen(Mon_Ascii[pointer][temp]);
            if (lenghtString == 12 && Mon_Ascii[pointer][temp][lenghtString - 1] != '\n')
                printf("\n");
        }
    }
    SetConsoleTextAttribute(color_Text, 7);
}

void screenRefresh() //My "cls"
{
    system("cls");
    printf("%*s", 35 - strlen(Mon_Names[current_Mon[1]]), "");
    printf("%s\n", Mon_Names[current_Mon[1]]);
    printf("%*s", 23, "");
    printHPNumber(current_Mon[1]); //Numbered HP Bar
    printf("\n%*s", 23, "");
    printHPBar(current_Mon[1]); //Colored HP bar
    printf("%*s", 23, "");
    printRedBall(6 - player_Faint[1]);
    printWhiteBall(player_Faint[1]);
    printf("\n");
    printASCII(current_Mon[1]);
    printASCII(current_Mon[0]);
    printf("%s\n", Mon_Names[current_Mon[0]]);
    printHPNumber(current_Mon[0]);
    printf("\n");
    printHPBar(current_Mon[0]);
    printWhiteBall(player_Faint[0]);
    printRedBall(6 - player_Faint[0]);

    //todo rewrite tis so the screen doesn't flicker (also cls is just not elegant)
}

float typeEffectiv(int atkType, int defType)
{
    if (type_Chart[defType][atkType] > 2)
    {
        //"return 1/(typeMultiplier - 1)" doesnt work so we use the conventional way
        float dividedMult = 1;
        dividedMult /=  (type_Chart[defType][atkType] - 1);
        return dividedMult;
    }
    else
        return type_Chart[defType][atkType];
}

void attackMon(int atkingMon, int defendingMon)
{
    int attackType, appliedATK, appliedDEF;

    //Determining the attack's type (first or second one) and its attribute (water, fire...)
    if (Mon_Type2[current_Mon[atkingMon]] != 18 && (rand() % 2) == 1) //18 is "Typeless"
        attackType = Mon_Type2[current_Mon[atkingMon]];
    else
        attackType = Mon_Type1[current_Mon[atkingMon]];

    //Determining type effectiveness
    float typeMult = typeEffectiv(attackType, Mon_Type1[current_Mon[defendingMon]]);
    if (Mon_Type2[current_Mon[defendingMon]] != 18) //18 is "Typeless"
        typeMult *= typeEffectiv(attackType, Mon_Type2[current_Mon[defendingMon]]);

    //Determining if the attack is physical or special
    int atkCatFlag = rand() % 2;
    if (atkCatFlag == 0) //0 = physical, 1 = special
    {
        appliedATK = Mon_ATK[current_Mon[atkingMon]];
        appliedDEF = Mon_DEF[current_Mon[defendingMon]];
    }
    else
    {
        appliedATK = Mon_SPA[current_Mon[atkingMon]];
        appliedDEF = Mon_SPD[current_Mon[defendingMon]];
    }

    /*Damage = ({[(2xattcLevel/5)+2] * AttackDamage * yourAtk/oppDef/50}+2) * Modifiers...
    Modifiers = numTargets * Weather * Badge * Crit * random(0.85 to 1.00) * STAB * TypeEffct * Burn * sizeMult
    Level is 100, AttackDamage is 70, STAB is 1.5, TypeEffct and random varies
    Ignores numTargets, Weather, Badge, Crit(!), Burn, sizeMult*/
    int randomvalue = 85 + rand() % 16;
    //printf("\nappliedATK = %d, appliedDEF = %d, randomvalue = %d", appliedATK, appliedDEF, randomvalue);
    float damageFl = ((58.8 * appliedATK / appliedDEF) + 2) * randomvalue * 0.015 * typeMult;
    int damageInt = damageFl;
    if (damageInt == 0)
        damageInt = 1;
    else if (damageInt > Mon_HPleft[current_Mon[defendingMon]]) //for damage display purposes, uneeded otherwise
        damageInt = Mon_HPleft[current_Mon[defendingMon]];

    Mon_HPleft[current_Mon[defendingMon]] -= damageInt;
    if (Mon_HPleft[current_Mon[defendingMon]] == 0)
        knockout_Flag = defendingMon;

    screenRefresh();
    printf("\n%s uses ", Mon_Names[current_Mon[atkingMon]]);
    printAtk(attackType);
    if (atkCatFlag == 0)
        printf(" Punch");
    else
        printf(" Beam");
    SetConsoleTextAttribute(color_Text, 7); //Defaults text color
    printf("!");
    if (typeMult > 1)
        printf("\nIT'S SUPER EFFECTIVE!");
    else if (typeMult < 1)
        printf("\nIt's not very effective...");
    //printf("\n%f", typeMult); tests multiplier
    printf("\n%s takes %d damage.", Mon_Names[current_Mon[defendingMon]], damageInt);
    if (knockout_Flag != 2)
        printf("\n%s has fainted!", Mon_Names[current_Mon[defendingMon]]);

    printf("\nPress any key to continue...");
    getch();
}

void randomSend(int pointer)
{
    if (player_Faint[pointer] != 6)
    {
        int convertedPointer = pointer * 6;
        int randomSnd;
        do{
            randomSnd = convertedPointer + rand() % 6;
        }while (Mon_HPleft[randomSnd] == 0);

        current_Mon[pointer] = randomSnd;
    }
    //todo make this less awful
}

void knockoutMon(int faintedMon)
{
    knockout_Flag = faintedMon;
    player_Faint[knockout_Flag]++;
    randomSend(knockout_Flag);
}

void attackTurn()
{
    if ((knockout_Flag) == 2)
    {
        int fasterMon;
        if (Mon_SPE[current_Mon[0]] > Mon_SPE[current_Mon[1]])
            fasterMon = 0;
        else if (Mon_SPE[current_Mon[0]] < Mon_SPE[current_Mon[1]])
            fasterMon = 1;
        else
            fasterMon = rand() % 2;

        int slowerMon = abs(fasterMon - 1); //(abs -1) switces between 0 to 1 and vice-versa
        attackMon(fasterMon, slowerMon);
        //printf("\nspeed_Check = %d", fasterMon);
        //printf("\nEita...\nMon_HPleft[current_Mon[fasterMon]] = %d\nMon_HPleft[current_Mon[slowerMon]] = %d\ntesteAbs = %d", Mon_HPleft[current_Mon[speed_Check]], Mon_HPleft[current_Mon[abs(speed_Check) - 1]], testeAbs);
        if (Mon_HPleft[current_Mon[slowerMon]] > 0) //Checks if "slower" mon is still alive
        {
            attackMon(slowerMon, fasterMon);
            if (Mon_HPleft[current_Mon[fasterMon]] == 0) //If fainted
                knockoutMon(fasterMon);
        }
        else
            knockoutMon(slowerMon);
    }
    else
    {
        screenRefresh();
        printf("\nPlayer %d sends out %s!", knockout_Flag + 1, Mon_Names[current_Mon[knockout_Flag]]);
        knockout_Flag = 2;
        getch();
    }
}

char betChoice()
{
    printf("\n");
    char charChoice;
    do
    {
        printf("Which player do you bet? Press 1 or 2 to the corresponding Player: ");
        scanf(" %c", &charChoice);
        if (charChoice != '1' && charChoice != '2')
            printf("Invalid choice, please type again.\n");
    }
    while (charChoice != '1' && charChoice != '2');
    return charChoice;
}

void monAssign(int position)
{
    int tempArray[6];
    //Assigning Mons to Chosen Trainer
    for (int count = 0; count < 6; count++)
    {
        int random_Mon = 1 + rand() % 151;

        //Species Clause (players won't have the same Mon twice)
        int speciesFlag = 0; //boolean flag
        for (int checkRandom = 0; checkRandom < count; checkRandom++)
        {
            if (tempArray[checkRandom] == random_Mon)
                speciesFlag = 1;
        }
        if (speciesFlag == 0)
            tempArray[count] = random_Mon;
        else
            count--;
    }

    //Sorting Mons so fopen is more efficient. Fopen will run twice for each player but could be done once with more code
    int compara (const void * a, const void * b)
    {
      return ( *(int*)a - *(int*)b );
    }
    qsort(tempArray, 6, sizeof(int), compara);

    int tempIndex = 0;
    for (int count = position; count < position + 6; count++)
    {
        players_Mons[count] = tempArray[tempIndex];
        tempIndex++;
    }
}

void playersGetMons()
{
    monAssign(0);
    monAssign(6);
}

void dataAssign(int position) //0 = player 1, 6 = player 2
{
    //Assigning Mon stats with fopen
    FILE *txtData = fopen("data.txt", "r");
    if (!txtData)
    {
        printf("Could not open data.txt, exiting program.");
        exit(EXIT_FAILURE);
    }

    else
    {
        char c;
        int lineCounter = 1;
        for (int monCounter = position; monCounter < position + 6; monCounter++)
        {
            while (players_Mons[monCounter] != lineCounter) //Searches for Mon's data in the file
            {
                while((c = getc(txtData)) != '\n'){} //Line skip
                lineCounter++;
            }

            char dataset[10];

            strcpy(Mon_Names[monCounter], "");
            for (int dataCounter = 0; dataCounter < 10; dataCounter++){ //Assign NAME
                char txtCharacter[2]; //[2] is needed for strcat
                txtCharacter[0] = getc(txtData);
                if (txtCharacter[0] != ' ')
                {
                    txtCharacter[1] = '\0';
                    strcat(Mon_Names[monCounter], txtCharacter);
                }
            }

            for (int dataCounter = 0; dataCounter < 3; dataCounter++) //Assign HP
                dataset[dataCounter] = getc(txtData);
            //HP Formula = ({[IV+2*Base Stat+([EVs]/4)+100] * Level}/100)+Level+10, there is no IV/EV, level is 100
            Mon_HP[monCounter] = 100*(dataset[0] - '0') + 10*(dataset[1] - '0') + (dataset[2] - '0');
            Mon_HPleft[monCounter] = Mon_HP[monCounter] = 2*Mon_HP[monCounter]+110;

            for (int dataCounter = 0; dataCounter < 3; dataCounter++) //Assign ATK
                dataset[dataCounter] = getc(txtData);
            //Other Stats Formula = [({[IV+2*Base Stat+([EVs]/4)+100] * Level}/100)+5]*nature, there is no IV/EV/nature, level is 100
            Mon_ATK[monCounter] = 100*(dataset[0] - '0') + 10*(dataset[1] - '0') + (dataset[2] - '0');
            Mon_ATK[monCounter] = 2*(Mon_ATK[monCounter])+5;

            for (int dataCounter = 0; dataCounter < 3; dataCounter++) //Assign DEF
                dataset[dataCounter] = getc(txtData);
            Mon_DEF[monCounter] = 100*(dataset[0] - '0') + 10*(dataset[1] - '0') + (dataset[2] - '0');
            Mon_DEF[monCounter] = 2*(Mon_DEF[monCounter])+5;

            for (int dataCounter = 0; dataCounter < 3; dataCounter++) //Assign SPA
                dataset[dataCounter] = getc(txtData);
            Mon_SPA[monCounter] = 100*(dataset[0] - '0') + 10*(dataset[1] - '0') + (dataset[2] - '0');
            Mon_SPA[monCounter] = 2*(Mon_SPA[monCounter])+5;

            for (int dataCounter = 0; dataCounter < 3; dataCounter++) //Assign SPD
                dataset[dataCounter] = getc(txtData);
            Mon_SPD[monCounter] = 100*(dataset[0] - '0') + 10*(dataset[1] - '0') + (dataset[2] - '0');
            Mon_SPD[monCounter] = 2*(Mon_SPD[monCounter])+5;

            for (int dataCounter = 0; dataCounter < 3; dataCounter++) //Assign SPE
                dataset[dataCounter] = getc(txtData);
            Mon_SPE[monCounter] = 100*(dataset[0] - '0') + 10*(dataset[1] - '0') + (dataset[2] - '0'); //No need to convert speed

            //TYPE ASSIGN NOW
            Mon_Type1[monCounter] = getc(txtData) - 48; //A = 65 in ASCII, turns into 10; 6 = ASCII 54, turns into 6
            if (Mon_Type1[monCounter] > 9)
                Mon_Type1[monCounter] -= 7;
            Mon_Type2[monCounter] = getc(txtData) - 48;
            if (Mon_Type2[monCounter] > 9)
                Mon_Type2[monCounter] -= 7;
        }

        //printing mon names
        int indexCounter = 0;
        for (int monCounter = position; monCounter < position + 6; monCounter++)
        {
            SetConsoleTextAttribute(color_Text, 15);
            printf("%s", Mon_Names[monCounter]);
            SetConsoleTextAttribute(color_Text, 7);
            printf(", HP %d, ", Mon_HP[monCounter]);

            //printf("Mon %d: %s, HP %d, ATK %d, DEF %d, SPA %d, SPD %d, SPE %d, Type ", indexCounter + 1, Mon_Names[monCounter], Mon_HP[monCounter], Mon_ATK[monCounter], Mon_DEF[monCounter], Mon_SPA[monCounter], Mon_SPD[monCounter], Mon_SPE[monCounter]);
            printAtkThenColorless(Mon_Type1[monCounter]); //Calls Mon's type with a colored font
            if (Mon_Type2[monCounter] != 18)
            {
                printf("/");
                printAtkThenColorless(Mon_Type2[monCounter]);
            }
            printf("\n");
            indexCounter++;
        }
    }
    fclose(txtData);

    //ascii sprites
    FILE *asciiData = fopen("ascii.txt", "r");
    if (!asciiData)
    {
        printf("Could not open ascii.txt, exiting program.");
        exit(EXIT_FAILURE);
    }

    int lineCounter = 0;
    for (int monCounter = position; monCounter < position + 6; monCounter++)
    {
        char c;
        int asciiFinder = (players_Mons[monCounter] - 1) * 5; //5 lines of data for each mon
        while (asciiFinder != lineCounter) //Searches for mon's data in the file
        {
            while((c = getc(asciiData)) != '\n')
            {} //Line skip
            lineCounter++;
            //printf("players_Mons[monCounter] = %d, asciiFinder = %d, lineCounter = %d\n", players_Mons[monCounter], asciiFinder, lineCounter);
        }

        //ASCII COLORS
        c = getc(asciiData);
        for (int temp = 0; temp < 4; temp++)
        {
            if (c != '\n')
            {
                if (c == ' ')
                {
                    if (temp == 0)
                        Mon_SpriteColors[monCounter][0] = 7; //7 = colorless
                    else
                        Mon_SpriteColors[monCounter][temp] = Mon_SpriteColors[monCounter][temp - 1];
                }
                else
                {
                    Mon_SpriteColors[monCounter][temp] = c - 48; //A = 65 in ASCII, turns into 10; 6 = ASCII 54, turns into 6
                    if (Mon_SpriteColors[monCounter][temp] > 9)
                        Mon_SpriteColors[monCounter][temp] -= 7;
                }
                //if (c < 0 && c > 15)
            c = getc(asciiData);
            }
            else
            {
                while (temp < 4)
                {
                    if (temp == 0)
                        Mon_SpriteColors[monCounter][0] = 7; //7 = colorless
                    else
                        Mon_SpriteColors[monCounter][temp] = Mon_SpriteColors[monCounter][temp - 1];
                    temp++;
                }
            }
        }
        while (c != '\n')
            c = getc(asciiData);

        //ASCII SPRITE
        for (int temp = 0; temp < 4; temp++) //4 = 4 lines
        {
            for (int erasePointer = 0; Mon_Ascii[monCounter][temp][erasePointer] != 0 && erasePointer < 12; erasePointer++) //cleans content for subsequent matches
                Mon_Ascii[monCounter][temp][erasePointer] = 0;
            char txtCharacter;
            strcpy(Mon_Ascii[monCounter][temp], "");
            int charlimit = 0;
            do
            {
                txtCharacter = getc(asciiData);
                if (charlimit < 12) //12 characters per line only
                {
                    Mon_Ascii[monCounter][temp][charlimit] = txtCharacter;
                    charlimit++;
                }
            }
            while (txtCharacter != '\n');
        }
        lineCounter+=5;
    }
    fclose(asciiData);
}

void monsGetData()
{
    SetConsoleTextAttribute(color_Text, 11);
    printf("PLAYER 1\n");
    SetConsoleTextAttribute(color_Text, 7);
    dataAssign(0);
    SetConsoleTextAttribute(color_Text, 12);
    printf("\nPLAYER 2\n");
    SetConsoleTextAttribute(color_Text, 7);
    dataAssign(6);
}

void typeChart()
{
    FILE *txtData = fopen("typeeffectiv.txt", "r");
    char c;
    int xAxis = 0, yAxis = 0;
    for (int counter = 0; counter != 341; counter++) //341 = 19 * 18 - 1
    {
        c = getc(txtData);
        if (c != '\n')
        {
            type_Chart[xAxis][yAxis] = c - '0'; //converts ASCII to int
            if (xAxis == 17)
            {
                xAxis = 0;
                yAxis++;
            }
            else
                xAxis++;
        }
    }
    fclose(txtData);
}

void main()
{
    srand(time(NULL)); //For Random generation
    color_Text = GetStdHandle(STD_OUTPUT_HANDLE);
    typeChart(); //Reading type effectiveness chart

    //window size
    HANDLE winSizeHndl = GetStdHandle(STD_OUTPUT_HANDLE);

    COORD windowsBox;
    windowsBox.X = 36;
    windowsBox.Y = 21;

    SMALL_RECT srctWindow;
    srctWindow.Top = 0;
    srctWindow.Bottom = windowsBox.Y - 1;
    srctWindow.Left = 0;
    srctWindow.Right = windowsBox.X - 1;

    SetConsoleWindowInfo(winSizeHndl, TRUE, &srctWindow);
    SetConsoleScreenBufferSize(winSizeHndl, windowsBox);

    HWND consoleWindow = GetConsoleWindow();
    SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
    //end of window size

    char exit;
    do
    {
        playersGetMons();
        monsGetData();
        char player_Bet = betChoice();
        knockout_Flag = 2;
        player_Faint[0] = player_Faint[1] = 0;

        printf("\nBATTLE WILL BEGIN SOON. GOOD LUCK");
        //Sleep(2000);

        randomSend(0); //sets Mons to send
        randomSend(1);

        screenRefresh(); //THIS IS THE SCREEN
        printf("\nPlayer 1 sent out %s!\nPlayer 2 sent out %s!", Mon_Names[current_Mon[0]], Mon_Names[current_Mon[1]]);
        getch();

        while(player_Faint[0] != 6 && player_Faint[1] != 6) //while both players have mons left
            attackTurn();

        screenRefresh();
        char idWinner;
        printf("\nEND OF MATCH.");
        if (player_Faint[1] == 6)
        {
            idWinner = '1';
            printf("\nPLAYER 1 WON!");
        }
        else
        {
            idWinner = '2';
            printf("\nPLAYER 2 WON!");
        }

        if (idWinner == player_Bet)
            printf("\nCongrats, you bet right!");
        else
            printf("\nBetter luck next time...");

        do
        {
            printf("\nPlay again? (1 = yes, 2, no): ");
            scanf(" %c", &exit);
            if (exit != '1' && exit != '2')
                printf("Invalid choice.");
        }while (exit != '1' && exit != '2');

        if (exit == '1')
            system("cls");

    }while (exit == '1');

    printf("\n\nPress any key to exit...");
    getch();
}
