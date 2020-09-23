#include "mbed.h"

DigitalOut ouvrir(D2); //pin3
DigitalOut fermer(D3); //pin4
Serial bluetooth(D5,D4); //module bluetooth HC-06 en communication sur le Serial1 de la carte
DigitalOut gyro(D6); //fil vert
InterruptIn finAv(D7); //fil noir
InterruptIn finAr(D8); //fil bleu
InterruptIn buttonClose(D9); //fil vert
InterruptIn buttonOpen(D10); //fil orange
InterruptIn fence(D11); //barrière infra, fil violet
DigitalOut marche(D12); //pin2, ENA
DigitalOut buzzer(D13); //fil violet

int etatfinAr=0;
int etatfinAv=0;
int etatbuttonOpen=0;
int etatbuttonClose=0;

Timer t1; 
Ticker flipper;

//flip l'état de la led et du buzzer
void flip() {
    gyro = !gyro;
    buzzer = !buzzer;
}

//fonction allumage/extinction de la led en fonction de l'état précédent
void Light()
{
    gyro = gyro ^1; 
}

//fonction allumage/extinction du buzzer en fonction de l'état précédent
void Ring()
{
    buzzer = buzzer ^ 1; 
}

//interrup fin Arrière
void pressedfinAr()
{
    etatfinAr=1;
}

void releasedfinAr()
{
    etatfinAr=0;
}

//Interrupt fin Avant
void pressedfinAv()
{
    etatfinAv=1; 
}

void releasedfinAv()
{
    etatfinAv=0;
}

//Interrupt buton ouvrir
void pressedbuttonOpen()
{
    etatbuttonOpen=1; 
}

void releasedbuttonOpen()
{
    etatbuttonOpen=0;
}

//Interrupt button fermer
void pressedbuttonClose()
{
    etatbuttonClose=1; 
}

void releasedbuttonClose()
{
    etatbuttonClose=0;
}

void OpenS()//ouverture portail petit pour moto, vélo ou piéton (uniquement via application android)
{
    wait(1); //attend 1 seconde (relacher le bouton) pour éviter le conflit avec la boule while après
    flipper.attach(&flip, 0.25); //clignotement de la led et du buzzer
    t1.start(); //timer pour temps d'ouverture du portail
    while(etatfinAr == 0 && etatbuttonOpen==0 && etatbuttonClose ==0 && t1.read()<1) //s'ouvre pendant une seconde si pas d'interruption
    {
        marche =1; //roule
        ouvrir=1; //dans le sens ouverture
        fermer=0; 
    }
        marche=0; //arrêt
        flipper.detach(); //stop le clignotement de la led et du buzzer
        buzzer=0; //forcer le buzzer à 0
        gyro=0; //forcer le gyro à 0
        wait(0.5); //pause de 0.5s (relacher le bouton si appui) pour éviter le commencement d'une nouvelle fonction non souhaité ici
        t1.stop(); //arrêt du timer
        t1.reset(); //RAZ du timer
}

void OpenL() //ouverture totale pour voiture (via bouton physique ou application android)
{
    wait(1); //attend 1 seconde (relacher le bouton) pour éviter le conflit avec la boule while après
    flipper.attach(&flip, 0.25); //clignotement de la led et du buzzer
    while(etatfinAr == 0 && etatbuttonOpen==0 && etatbuttonClose ==0) //tant que le portail n'a pas appuyé sur la buté ou interruption
    {
        marche =1; //roule
        ouvrir=1; //dans le sens ouverture
        fermer=0;
    }
        marche=0; //arrêt
        flipper.detach(); //stop le clignotement de la led et du buzzer
        buzzer=0; //forcer le buzzer à 0
        gyro=0; //forcer le gyro à 0
        wait(0.5); //pause de 0.5s (relacher le bouton si appui) pour éviter le commencement d'une nouvelle fonction non souhaité ici
}

//fermeture totale (via bouton physique ou application android)
void Close()
{
    wait(1); //attend 1 seconde (relacher le bouton) pour éviter le conflit avec la boule while après
    flipper.attach(&flip, 0.25); //clignotement de la led et du buzzer
    while(etatfinAv == 0 && etatbuttonOpen==0 && etatbuttonClose ==0) //tant que le portail n'a pas appuyé sur la buté ou interruption
    {
        marche =1; //roule
        ouvrir=0;
        fermer=1; //dans le sens fermeture
    }
    marche=0; //arrêt
    flipper.detach(); //stop le clignotement de la led et du buzzer
    buzzer=0; //forcer le buzzer à 0
    gyro=0; //forcer le gyro à 0
    wait(0.5); //pause de 0.5s (relacher le bouton si appui) pour éviter le commencement d'une nouvelle fonction non souhaité ici
}

int main()
{
    //fin de course arrière
    finAr.fall(&releasedfinAr);
    finAr.rise(&pressedfinAr);
    finAr.mode(PullDown); //force le bouton à 0
    //fin de course avant
    finAv.fall(&releasedfinAv);
    finAv.rise(&pressedfinAv);
    finAv.mode(PullDown); //force le bouton à 0
    
    //ouvrir
    buttonOpen.fall(&releasedbuttonOpen);
    buttonOpen.rise(&pressedbuttonOpen);
    buttonOpen.mode(PullDown); //force le bouton à 0
    
    //fermer
    buttonClose.fall(&releasedbuttonClose);
    buttonClose.rise(&pressedbuttonClose);
    buttonClose.mode(PullDown); //force le bouton à 0
    
    bluetooth.baud(9600); //on règle le débit de la liaison série bluetooth sur 9600
    Close(); //on initialise la position du portail en fermé en forcant la fermeture à chaque allumage
    
    while (1) {
        
        //Bluetooth
        if(bluetooth.readable()) { //on regarde si le bluetooth envoi quelque chose
            char rx =bluetooth.getc(); //on stock le caractère recu dans la variable rx
            switch (rx) { //on crée un swith avec plusieurs cas pour les différents caractères possibles envoyés par le bluetooth
                case 'W': //appuie sur le bouton piéton
                    OpenS(); //lancement de la fonction petit ouverture
                    break; //sortie du switch
                case 'C': //appuie sur le bouton voiture
                    OpenL(); //lancement de la fonction ouverture totale
                    break; //sortie du switch
                case 'L': //appuie sur le bouton ampoule
                    Light(); //lancement de la fonction allumage de la LED
                    break; //sortie du switch
                case 'B': //appuie sur le bouton cloche
                    Ring(); //lancement de la fonction sonner le buzzer
                    break; //sortie du switch
                case 'X': //appuie sur le bouton fermer 
                    Close(); //lancement de la fonction fermeture
                    break; //sortie du switch
            }
        }
        
        //on veut ouvrir le portail
        if (etatbuttonOpen == 1) //apuie sur le bouton ouvrir
        {
            OpenL(); //lancement de la fonction ouverture totale
        }
        
        //on veut fermer le portail
        if (etatbuttonClose == 1) //apuie sur le bouton ouvrir
        {
            Close(); //lancement de la fonction fermeture
        }
    }
}