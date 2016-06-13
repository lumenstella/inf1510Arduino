/* Egne notater
Koblinger:                          Se i drive "koblinger"

Info
    - Reduser strings i utskrift for aa redusere minnebruk.

  "//TODO" skrives der vi maa gjoere noe
  - tilt for paa -test
  - Fjern //Serial.print's naar testet
  - lage lydfil med bakgrunnshistorie
  - lage lydfil som sier hva man skal gjøre om man sitter fast. 
*/

/* Dette programmet styrer en bamse som hjelper barn med aa lese tekster.
 * 
 * @author  http://www.uio.no/studier/emner/matnat/ifi/INF1510/v16/prosjekter/karoshi/
 * @version 0.98
 * @since   2016-10-6 22:48
  
Forkortelser:
    VRM = Voice Recognition Module
    
VRM dokumentasjon:                  http://www.geeetech.com/wiki/index.php/Arduino_Voice_Recognition_Module
	og:                  	            http://www.geeetech.com/wiki/images/6/69/Voice_Recognize_manual.pdf

MP3-modul dokumentasjon:            http://www.geeetech.com/vs1053-mp3-breakout-board-sd-card-slot-p-611.html
  wiki:                             http://www.geeetech.com/wiki/index.php/VS1053_MP3_breakout_board_with_SD_card
  bibliotek:                        https://github.com/madsci1016/Sparkfun-MP3-Player-Shield-Arduino-Library
  fungerende eksempel og metoder:   http://www.billporter.info/2012/01/28/sparkfun-mp3-shield-arduino-library/comment-page-2/

Mikrofon-modul dokumentasjon:       http://www.ebay.com/itm/Microphone-Sensor-High-Sensitivity-Sound-Detection-Module-For-Arduino-/201052209891?hash=item2ecfa542e3:g:pIoAAOSwPe1T6h1W

Pushbutton dokumentasjon:           https://www.arduino.cc/en/tutorial/pushbutton



Mulig input fra bruker:
	Kommando 1 er "Hjelp"
	Kommando 2 er "ferdig"
	Kommando 3 er "Boknavn"
	Kommando 4 er "Oversett"
	Kommando 5 er "Natta"'
  Kommando 6 er "Hei" 
  Kommando 7 er "Skru på/av"
 

  

Om lydfilene paa SD-Kortet:
    - Alle maa hete "trackXYZ.mp3"
    - En spiller av lydfil ved aa kalle MP3player.playTrack(XYZ);
    - Metoden returnerer en int som feilkode (det er ingen dokumentasjon for disse med kortet, men vi har kommet frem til foelgende oversikt gjennom testing)
        * 0 tilsvarer ingen feil
        * 1 tilsvarer at filen ikke ble spillt av siden en annen fil fortsatt var under avspilling
        * 2 tilsvarer at filen ikke finnes
        * ...
        * 6 tilsvarer at MP3-kortet ikke ble satt opp ordentlig 
        * andre feilkoder over 2 er ukjente

Organisering, format for lydfilene paa SD-kortet: //TODO(Ved å organisere kan vi gi instruks til laerere for aa legge inn nye boeker, og de blir ogsaa brukere)
    1 er "Hei".
  2 er "hva vil du lese?".
  3 er "Ja den kan vi lese.".
  4 er "Nei tror ikke vi kan lese den. den har jeg ikke øvd på"
  5 er "Vil du ha hjelp?" eller "Snakk til meg hvis du står fast, si Hjelp hvis du vil jeg skal lese setningen, Oversett hvis jeg skal oversette, eller Natta hvis du ikke vil lese lenger."  
  6 er "Jeg legger meg til å sove igjen, God natt"
  7 er "Da har vi lest alt!"
  8 er "Du må velge en bok først!"
  9 er "du kan lese nå"/"kan du lese nå?"       Vi gaar vekk fra "din tur" siden de ble irritert av det
  10 er "Hva heter boken?"
  11 er "Jeg trodde jeg kunne lese denne boken, men det er for vanskelig for meg. Kan du be laereren din om aa laere meg?" (Sies naar boken ikke er satt opp riktig, og en foresatt maa fikse det).
  12 er "Noe foeles litt galt, jeg tror ikke jeg kan lese naa. Kan du spoerre en voksen om aa hjelpe?" (Sies naar bamsen ikke er satt opp riktig, og en foresatt maa fikse det).
  13 er "kan du starte?" (Sier foer boken begynes aa leses).
  14 er "her staar det-" (Sies foer opplesning av tekst).
  15 er "dette betyr-"  (Sies foer oversettelse av tekst).

Med dette oppsettet kan en bare ha en bok. Hvis vi vil legge til flere trenger vi aa sette av en kommando for hvert boknavn, og aa sette av lydfilplasser for boekene, f.eks bok1 har 101-199, bok2: 201-299 osv. Ved at det er en lydfilplass som ikke er tildelt mellom hver bok, slipper en aa sjekke om en er innenfor bokens grenser osv
        101 er foerste setning fra boka
        102 er andre setning fra boka
        103 osv.....
        
        201 er foerste setning oversatt fra boka
        202 er andre setning oversatt fra boka
        203 osv.....
*/

//Foelgende er MP3-relatert
#include <SPI.h>                                                //Importer protokollen mikrokontrolleren bruker med modulene.
#include <SdFat.h>                                              //Importer bilioteket noedvendig for aa gjoere SD kort lesing.
#include <SdFatUtil.h>                                          //Importer det andre bilioteket noedvendig for aa gjoere SD kort lesing.
#include <SFEMP3Shield.h>                                       //Importer bilioteket noedvendig for aa bruke MP3 modulen.
SdFat sd;                                                       //Sdkortet. Merk: Denne linjen maatte legges til siden det ikke finnes et oppdatert MP3-bibliotek.
SFEMP3Shield MP3player;                                         //selve MP3 spilleren.

//Foelgende variabler er VRM-relatert
  byte com = 0;                                                 //VRM output lagres her.

//Foelgende variabler er eget
  //Preferanser
#define debugPaa true                                           //Slaa paa seriell utskrift for aa bidra til aa vise hvordan systemet jobber naar true.
#define onPin 4                                                 //Paa-knapp plasseringen.
#define micPin 0                                                //Mikrofon plasseringen.
#define heiDur 400                                              //Hvor lang til skal systemet vente foer en kan spille av en lydfil etter "hei" lydfilen.
#define gyldigDur 1500                                          //Hvor lang til skal systemet vente foer en kan spille av en lydfil etter lydfil nr 3.
#define setningPause 200                                        //Naturlig pause mellom hver lydfil-avspilling for aa etterlikne ekte tale.
#define tiltPin 5                                               //Tilt-sensor-plasseringen.
#define oppreist true                                           //Verdien til tilt-sensoren naar bamsen sitter i riktig(paa-posisjonen) posisjon.
#define mode "ANNENHVER"                                        //Mode kan vaere "ANNENHVER", "SYSBRUKER", eller "BRUKERSYS". Hvis "ANNENHVER", vil bruker og system lese annenhver gang. For "BRUKERSYS" vil brukeren lese setninger foerst, og så vil systemet gjenta. For "SYSBRUKER"  vil systemet lese setninger, og brukeren vil gjenta foer de gaar videre.
#define foersteMaseTid 22000                                    //Dette er den minste tiden vi maa vente foer systemet kan be brukeren om brukeren vil ha hjelp.
#define faktor 1.3                                              //Denne multipliserer vente til mellom hver gang systemet spoerr om brukeren vil ha hjelp/forklaringer. Hoyere verdier gir et mindre "masete" system.
#define minMargin 1                                             //Dette er med paa aa definerer hva som er stillhet, og hva som ikke er det. Hoeyere vil kreve mindre stillhet, enn lavere verdier.
#define marginPart 20                                           //Dette er med paa aa definerer hva som er stillhet, og hva som ikke er det. Hoeyere vil kreve mer stillhet, enn lavere verdier. Ikke sett til 0.
#define avbryt false                                            //Hvis true: Systemet vil slutte aa snakke for aa si svare bruker med en gang.
#define ventHeller false                                        //Hvis true, og avbryt==false: Systemet vil ikke avbryte setninger for aa svare brukeren, men heller snakke ferdig foerst.
#define ikkeKoe true                                            //Hvis true, og avbryt==false, og ventHeller==false: Systemet vil ikke sette komandoer i koe, men heller snakke ferdig foerst. Vi fant ut gjennom brukertesting at aa sette komandoer i koe ble forvirrende for brukeren.
#define lenge 600000//10 min0                                   //Hvis bamsen ligger stille "lenge" i "av" tilstand, vil den skru seg paa hvis tiltsensor endrer verdi.
#define sovTungt false                                          //Hvis vi oensker at bamsen aldri skal vaakne opp pga bevegelse (etter aa ha ligget stille lenge), sett vi denne til true.

 //Variabler
  int setningNr=101;                                            //Denne holder styr paa hvilken setning i boken vi har kommet til. Se oppsett ovenfor for mer info.

  boolean bokValgt=false;                                       //true naar bruker har valgt en bok.
  boolean treigTilAaLese=false;                                 //true naar systemet venter paa at bruker skal lese en setning.
  boolean treigtBokValg=false;                                  //true naar systemet venter paa at bruker skal velge bok.
  boolean fattHjelp=false;                                      //Hvis systemet har hjulpet brukeren med en fasit, trenger vi ikke gjenta fasiten igjen foer vi gaar videre. Denne er bare relevant naar mode==2.
  boolean tiltState=false;                                      //Holder styr paa om bamsen blir beveget mye paa 
  boolean exitPgaKnapp = false; //for wizzard 
  
  unsigned long ventetSiden=0;                                  //Denne oppdateres ("ventetSiden=millis();") naar systemet begynner aa vente paa input fra brukeren.
  unsigned long paaTideDur=foersteMaseTid;                      //Hvor lang tid skal systemet skal gi brukeren paa aa gi input.
  unsigned long stilleSiden=0;

void setup(){
  Serial.begin(9600);
  setPins();                                                    //Oversikt over pins/krets-oppsett, og anbefalt praksis
  setupMp3();                                                   //Gjoer klar MP3 modulen 
  setupVRM();                                                   //Gjoer klar VRM for bruker input
  skrivKommandoVindu();
  stallTillOnPress();                                           //Vent til bamsen sitter oppreist og paa-knappen blir trykket ned
}

void setPins(){                     
    pinMode(tiltPin,INPUT);                                     //Tilt-sensoren settes i riktig modus.
    pinMode(onPin,INPUT);                                       //Paa-knappen settes i riktig modus.
    pinMode(micPin, INPUT);
}

void setupVRM(){
  delay(2000);                                                  //VRMkortet trenger pause (verdi hentet fra nettet).
  Serial.write(0xAA);                                           //Gjoer VRM klar for kommando.
  Serial.write(0x37);                                           //Kommando: sett til compact mode. Egner seg under bruk for aa jobbe raskt.
  delay(2000);                                                  //VRMkortet trenger pause (verdi hentet fra nettet).
  Serial.write(0xAA);                                           //Gjoer VRM klar for kommando
  Serial.write(0x21);                                           //Gjoer klar kommando nr. 1-5, og lytt etter de.
  delay(2000);                                                  //VRMkortet trenger pause.
}

boolean setupMp3(){                 
  //skriv("sMp3", 0);
  sd.begin(SD_SEL, SPI_HALF_SPEED);                             //Start sd-kortlesing på halv fart for bedre ytelse.
  MP3player.begin();                                            //Start MP3 modulen.
 //skriv("sMp3 slutt", 10);
}

void stopMP3(){
  //skriv("stMp3", 0);
  MP3player.stopTrack();                                        //Stop avspilling av lydfil.
  //skriv("stMp3 slutt", 10);
}

int playTrackNum(int t){
  //skriv("Ptn", 0);
  if(debugPaa){
      //Serial.println(t);
      }
  if(avbryt){
    stopMP3();                                                  //Hvis MP3 modulen spiller av en lyd, maa vi stoppe den, for aa kunne spille av en ny.
    //skriv("avbroet", 1);
  } else if (ventHeller){                                       //Hvis MP3 modulen spiller av en lyd kan vi ogsaa heller vente til den lyden er ferdig med avspilling. Dette gjoeres i preferanser oeverst.
    while(MP3player.isPlaying()){
    delay(300);
    }
  } else if (ikkeKoe){
    if (MP3player.isPlaying()) {
      return 1;
    }
  }
  int feilkode=MP3player.playTrack(t);                          //Spill av lydfil nr. t. Invariant: lydfiler maa foelge formatet "track001.mp3", "track002.mp3" ... osv for at metoden skal virke.
  snakkFerdig();
  //skriv("ptn slutt", 10);
  return feilkode;
}


//----DE ULIKE KOMMANDOENE-----
void kommando1(){
    //skriv("K 1", 0);                                          //Brukeren har sagt "hjelp", eller setningen vi er paa skal leses opp.
    if(bokValgt){
      siHerStaarDet();
    }else{
      siVelg();
      resetMaseTid(); 
      return;
    }
    if(mode=="ANNENHVER"){                                      //Hvis systemet og brukeren skal lese annenhver setning
                                                                //Les sentningen systemet jobber med, og den brukeren jobber med 
       //skriv("hjelper m/1", 1);
       setningNr--;
     
       lesSetning(false);
       //skriv("Hjelper m/2 ", 1);
       lesNesteSetning(false);
    } else if(mode=="BRUKERSYS"||mode=="SYSBRUKER"){
                                                                //Les setningen vi jobber med
     lesSetning(true);
    } else {
      //skriv("ukjent modus", 1);
      fatalError(404);
    }
    resetMaseTid();                                             //Paa denne maaten speor systemet brukeren om forklaring eller hjelp er noedvendig til passende tid
    //skriv("k1 slutt", 10);
}

void kommando2(){                                               //Brukeren er har sagt ferdig.  
    //skriv("k2", 0);               
  if(mode=="ANNENHVER"){                                      //Hvis systemet og brukeren skal lese annenhver setning
      //skriv("A", 1);
     lesNesteSetning(true);
      setningNr++; //skriv("K2 oeker nr", 1);
  }else if (mode=="SYSBRUKER"){                              //Hvis brukeren skal lese etter systemet
      //skriv("sb", 1);
      lesNesteSetning(true);
    } else if (mode=="BRUKERSYS"){                              //Hvis systemet skal lese etter brukere
      //skriv("bs", 1);
      lesSetning(true);
      setningNr++; //skriv("K2 oeker nr", 1);
    } else {
      //skriv("Du satte systemet til en modus som ikke finnes", 1);
      fatalError(404);
    }
    if(bokValgt){
      siDinTur();
    }
    resetMaseTid();                                             //Paa denne maaten speor systemet brukeren om forklaring eller hjelp er noedvendig til passende tid            
    //skriv("k2 slutt", 10);
}

void kommando3(){                                               //Brukeren har sagt navnet paa boken som skal leses.
  //skriv("k3", 0);
  valgtBok();                                                   //Systemet begynner aa lese med brukeren.
  resetMaseTid();                                               //Paa denne maaten speor systemet brukeren om forklaring eller hjelp er noedvendig til passende tid
  //skriv("k3 slutt", 10);
}

void kommando4(){                                               //Brukeren har sagt "Oversett" og oensker hjelp med forstaaelse.
	//skriv("k4", 0);
                                                                //Systemet leser opp lydfilen som tilsvarer setningen brukeren har kommet til, men paa spraaket som brukeren forstaar.
    if(mode=="ANNENHVER"){                                      //Hvis systemet og brukeren skal lese annenhver setning
                                                                //oversett sentningen systemet jobber med, og brukeren jobber med
      oversett(true);  
    } else if(mode=="BRUKERSYS"||mode=="SYSBRUKER") {
                                                                //oversett setningen vi jobber med
    oversett(false); 
    }  else {
      //skriv("Du satte systemet til en modus som ikke finnes", 1);
      fatalError(404);
    }
     
    //-----
    resetMaseTid();   
     //Paa denne maaten speor systemet brukeren om forklaring eller hjelp er noedvendig til passende tid
    //skriv("kslutt", 10);
}

void kommando5(){                                               //Brukeren har sagt "natta". Dvs at bamsen ikke skal brukes paa en stund.
    //skriv("Kommando 5", 0);
    //TODO legg til aa spoerre om du er sikker paa at vi er ferdige
    siHade();                                                   //"skru av systemet"(egentlig bare i en slags ventemodus), gi feedback, og endre noedvendige variabler.
    resetMaseTid();                                             //Paa denne maaten speor systemet brukeren om forklaring eller hjelp er noedvendig til passende tid
    //skriv("kommando5 slutt", 10);
}

  
void oversett(boolean forrigeOgsaa){
  //skriv("Oversetter", 0);
  if(bokValgt){                                                 //Hvis brukeren har valgt en bok.
    siDetteBetyr();
  //skriv("Oversetter forrige", 1);
  if(forrigeOgsaa){
    if(playTrackNum(setningNr+99)>2){                           //hvis vi faar feil her, saa finnes ikke filen og vi trenger ikke gjoere noe, eller saa kan vi ikke fikse det
      fatalError(404);
    }
  }
  //skriv("Oversetter denne", 1);
  int errr=playTrackNum(setningNr+100);                         //Spill av riktig lydfil (se oppsett), og lagre eventuell feilkode.
    if(errr==2){                                                //Hvis lydfilen ikke finnes og dette ikke er oversetting av neste setning, er vi ferdige med aa lese boken. Invariant: lydfilene maa foelge oppsettet.
     // ferdigMedBok(true);                                     //Oppdater noedvenige variabler for aa reflektere situasjonen.
    } else if(errr>0){                                          //Noe annet gikk galt. Se "feilkoder" oeverst.
      fatalError(errr);                                         //Systemet maa startes paa nytt for aa fungere.
     
    }else {                                                     //filen ble spillt av riktig.
      treigTilAaLese=true;                                      //Hvis ingenting skjer paa en stund, er det brukeren som kan trenge hjelp med aa lese.
      ventetSiden=millis();                                     //Systemet oppdateres slik at det ikke spoer brukeren om brukeren vil ha hjelp for tidlig.
    }
    
  } else {                                                      //Hvis brukeren ikke har valgt en bok, forteller systemet at det maa bli gjort.
    siVelg();
  }
  //skriv("oversett slutt", 10);
}

void kommando6(){
  siHei();
}

void kommando7(){
  ferdigMedBok(false);                                        //Gjoer det som er noedvendig for aa vaere ferdig med boken. Uten aa si "bra jobba".
    treigTilAaLese=false;                                       //setter denne til false, siden det ikke er mulig at brukeren trenger hjelp til aa lese noe.
    treigtBokValg=false;                                        //setter denne til false, siden det ikke er mulig at brukeren trenger hjelp til velge bok.
    stilleSiden=millis();
    stallTillOnPress();  
}



void loop() { 

//skriv("loop", 0);
  unsigned long naa = millis();
  if(treigtBokValg||treigTilAaLese){                            //Systemet er i en tilstand hvor det venter paa input fra bruker.
    if(brukerenHolderPaaAaLese()){                              //Hvis brukeren proever aa lese, lagrer vi det. Slik at vi kan hindre at systemet spoer brukeren om brukeren vil ha hjelp, mens brukeren arbeider.
        ventetSiden=naa;                                        // -slik lagrer vi det.  
        //skriv("prat", 1); 
    }
    if(naa-ventetSiden>=paaTideDur){                            //Brukeren bruker mye tid paa aa gi input, gi feedback: forklar.
        utaalmodig();                                           //Forklar brukeren (Dette "gir" systemet visibility).
    }
    } else {
      paaTideDur=foersteMaseTid;                                //Hvis vi ikke venter paa input fra bruker senker vi tiden for aa spoerre om brukeren trenger hjelp til orginaltiden (foersteMaseTid).
  
  }
  //skriv("venter paa k",1);
    while(Serial.available()){                                  //Mens systemet faar input fra VRM.
      //skriv("Serial av.", 0);         
		  com = Serial.read();
		  switch(com){

    //49 --> utover vil si tallene paa data tastaturet som ble brukt under evalueringen siden VRM en ikke fungerte.
    //for aa bruke VRM en vil man istedet bruke 0x11, 0x12 osv. 
    
    //VRM kortet sende 0x11 til 0x15 naar nokkelord 1-5 blir sagt
    case 0x11: kommando1();
    break;
    
    case 0x12: kommando2();
    break;
    
    case 0x13: kommando3();
    break;
    
    case 0x14: kommando4();
    break;
    
    case 0x15: kommando5();
    break;
    
    //vi kaller kommando 1-7 ved aa sende tallene 1-7 fra PC
    case 49:   //bytter ut 0x11 med 49                                               //VRM gjennkjente lyd tilsvarende innspillt lyd paa plass 1.
		kommando1();                                                //Kjoer kommando 1.
		break;
		
		case 50:                                                  //VRM gjennkjente lyd tilsvarende innspillt lyd paa plass 2.
		kommando2();                                                //Kjoer kommando 2.
		break;

		case 51:                                                  //VRM gjennkjente lyd tilsvarende innspillt lyd paa plass 3.
		kommando3();                                                //Kjoer kommando 3.
		break;

		case 52:                                                  //VRM gjennkjente lyd tilsvarende innspillt lyd paa plass 4.
		kommando4();                                                //Kjoer kommando 4.
    break;
    
		case 53:                                                  //VRM gjennkjente lyd tilsvarende innspillt lyd paa plass 5.
		kommando5();	                                              //Kjoer kommando 5.
		break;	

    case 54:                                                    //Kjorer kommando 6
    kommando6();
    break;

    case 55:
    kommando7();
    break;

    default: 
      //skriv("ukjent:", 1);
      if(debugPaa){
      //Serial.println(com);
      }    
      //skriv("", 1);
    break;
		} 
    tomKoo();
    skrivKommandoVindu();
     
	//skriv("seral read", 10);
  }
  //tomKoo();                                                  //tommer koen slik at man ikke kan si mange ganger, gjor bare en ting til forste oppgave er ferdig.//Paa denne maaten speor systemet brukeren om forklaring eller hjelp er noedvendig til passende tid
    
  //skriv("loop ferdig",10);
}

void lesNesteSetning(boolean ferdigHvisError){                  //Systemet leser neste setning.               
  //skriv("lns", 0);
  if(bokValgt){                                                 //Hvis brukeren har valgt en bok, kan systemet lese setningen.
    setningNr++; //skriv("LNS oeker nr", 1);
    lesSetning(ferdigHvisError);
  } else {                                                      //Hvis bok ikke er valgt, saa forklarer vi at det er noedvendig til brukeren.
        siVelg();
    }
//skriv("lNS slutt", 10);
}

boolean lesSetning(boolean ferdigHvisError){                    //Les setningen, boolean'n er true hvis vi ikke skal gaa videre i boken etter aa ha lest setningen.
  //skriv("ls", 0);
  boolean ferdig=false;
  if(bokValgt){                                                 //Hvis brukeren har valgt en bok, kan systemet lese setningen.                                               //Maa vi oeke telleren som holder styr paa hvilken setning vi er paa.
  int errr=playTrackNum(setningNr);           
    //skriv("error:", 1);
    if(debugPaa){
      //Serial.println(errr);
      }
    if(errr==2&&ferdigHvisError){                               //Hvis lydfilen ikke finnes, er vi ferdige med aa lese boken. Invariant: lydfilene maa foelge oppsettet.
      ferdigMedBok(true);                                       //Oppdater noedvenige variabler for aa reflektere situasjonen.
      ferdig=true;
  } else if(errr>2&&ferdigHvisError){                           //Noe annet gikk galt.
      fatalError(errr);                                         //Systemet maa startes paa nytt for aa fungere.
    } else {                                                    //Filen ble spillt av riktig.
      treigTilAaLese=true;                                      //Hvis ingenting skjer paa en stund, er det brukeren som kan trenge hjelp med aa lese. 
      ventetSiden=millis();
    }
  } else {
    siVelg();                                                   //Hvis bok ikke er valgt, saa forklarer vi at det er noedvendig til brukeren.
  }
  //skriv("lS slutt", 10);
return ferdig;
}

void siHeiOgVelg(){
  //skriv("Sier hei", 0);
  playTrackNum(1);                                              //Systemet sier hei.
  playTrackNum(2);                                              //Systemet forklarer brukeren at vi maa velge hva vi vil lese.
  treigtBokValg=true;                                           //Systemet er i en tilstand hvor brukeren maa gi input om hvilken bok som skal leses. Det er mulig at brukeren trenger hjelp til velge bok.
  ventetSiden=millis();
  //skriv("sHei slutt", 10);
}

void siHei(){
  playTrackNum(1);
  ventetSiden=millis();
}


void siHade(){
    //skriv("Sier hade", 0);
    playTrackNum(6);                                            //Systemet sier hade.
    ferdigMedBok(false);                                        //Gjoer det som er noedvendig for aa vaere ferdig med boken. Uten aa si "bra jobba".
    treigTilAaLese=false;                                       //setter denne til false, siden det ikke er mulig at brukeren trenger hjelp til aa lese noe.
    treigtBokValg=false;                                        //setter denne til false, siden det ikke er mulig at brukeren trenger hjelp til velge bok.
    stilleSiden=millis();
    stallTillOnPress();                                         //vent til brukeren slaar paa bamsen igjen.
    //skriv("sHade slutt", 10);
}
void valgtBok(){                                                //Brukeren har valgt en gyldig bok.
  //skriv("ValgtBok", 0);
  if(bokValgt){

  } else {
    bokValgt=true;                              
    setningNr=101;                                              //Vi er paa foeste setning i boka (se oppsett).
    playTrackNum(3);                                            //Gi feedback om at en gyldig bok er valgt.
    if(mode=="BRUKERSYS"||mode=="ANNENHVER"){                   //Hvis systemet og brukeren skal lese annenhver setning.
      playTrackNum(13);                                         //Si at brukeren kan starte.
    } else if(mode=="SYSBRUKER"){
      lesSetning(true);
      siDinTur();
    } else {
      //skriv("Du satte systemet til en modus som ikke finnes", 1);
      fatalError(404);
    }

    
    treigtBokValg=false;                                        //Setter denne til false, siden det ikke er mulig at brukeren trenger hjelp til velge bok.
    treigTilAaLese=true;                                        //Setter denne til true, siden det er mulig at brukeren trenger hjelp til aa lese noe.
  }
    ventetSiden=millis();                                       //Oppdater slik at systemet ikke tror brukeren har vaert stille lenge.
    //skriv("valgtBok slutt", 10);
}

void ferdigMedBok(boolean siBraJobba){                          //Oppdater variabler slik at systemets tilstand reflekterer situasjonen.
//skriv("ferdigMedBok", 0);
  bokValgt=false;                             
  treigTilAaLese=false;                                         //setter denne til false, siden det ikke er mulig at brukeren trenger hjelp til aa lese noe.
  if(siBraJobba){
    playTrackNum(7);                                            //Gi feedback om at vi er ferdige med boken.
  }
  //skriv("ferdigMedBok slutt", 10);
}

void siVelg(){                                                  //Forklar brukeren at vi maa velge en bok foerst. Ved aa gjoere dette, sikkrer vi (nestem) at brukeren har boken foran seg naar brukeren skal lese.
  //skriv("siVelg", 0);
  int muligError=playTrackNum(8);                               //Spill av forklaringen.
  treigtBokValg=true;                                           //Setter denne til true, siden det er mulig at brukeren trenger hjelp til velge bok.
  ventetSiden=millis();                                         //Oppdater slik at systemet ikke tror brukeren har vaert stille lenge.
  if(muligError!=0){                                            //Her har noe gaatt galt.
    ventetSiden=0;                                              //Derfor sier vi til systemet at vi har ventet lenge paa input fra brukeren. Det vil gjoere at systemet gir feedback tilsvarende denne kommandoen.
  }
 // skriv("siVelg slutt", 10);
}

void utaalmodig(){                                              //Kalles hvis ikke noe har skjedd paa en stund.
//skriv("utaalmodig", 0);
    if(treigtBokValg){                                          //Hvis problemet er at en bok maa velges-
        playTrackNum(10);                                       // -forklarer vi det her.
        ventetSiden=millis();                                   //Oppdater slik at systemet ikke tror brukeren har vaert stille lenge.
    } else if (treigTilAaLese){                                 //Hvis problemet er at brukeren ikke faar lest, eller proever aa lese en setning.
        playTrackNum(5);                                        //Spoer vi om brukeren vil ha hjelp.
        ventetSiden=millis();                                   //Oppdater slik at systemet ikke tror brukeren har vaert stille lenge.
    }
    paaTideDur=paaTideDur*faktor;                               //Venter enda lengere foer systemet spoer om bruker vil ha hjelp.
  //skriv("utaalmodig slutt", 10);
}
void stallTillOnPress(){                                        //Vent til bamsen blir skrudd paa.
skriv("systemet venter paa signal", 0);
  tiltState=digitalRead(tiltPin);
  boolean exitPgaTilt=false;
  while(!digitalRead(onPin)&&!exitPgaTilt && !exitPgaKnapp){                     //Paa-knappen maa vaere trykket ned, og bamsen i oppreist posisjon.
    if(Serial.available()){
       byte sendt = Serial.read();
       if(sendt == 55){
        exitPgaKnapp=true;
       }
    }
    delay(200);  
    if(slaaPaaPgaTilt()){
      
        exitPgaTilt=true;
        //skriv("vaaknet pga tilt", 1); 
    }
    //Serial.print("_");
  }
  exitPgaKnapp=false;
  
  //Serial.print("_/");
  siHeiOgVelg();                                                      //Gi feedback om at systemet har startet.
  skriv("systemet er paa", 10);
}

boolean brukerenHolderPaaAaLese(){                              //Sjekk om brukeren proever/arbeider, hvis ikke returner false.
   //skriv("brukerHolderPaaAaLese", 0);
                                                                //Hvis lydnivaaet er under grensen(justeres manuelt/fysisk paa potensiometer), sier vi at brukeren ikke arbeider, ved aa returnere false.
   //skriv("brukerenHolderPaaAaLese slutt", 10);
   return analogRead(micPin)<1000;                              //"23","850","950" tilsvarer prat, "1023" tilsvarer stillhet Dette fant vi ut av gjennom testing og dokumentasjon.
}

void ventTilDetErStille(){
  while(brukerenHolderPaaAaLese()){
    delay(200);
  }
  unsigned long stilleMillis = millis();
  boolean fortsett= true;
  while(fortsett){
    if(brukerenHolderPaaAaLese()){                              // skal gjore noe om d ikke er stille.
      ventTilDetErStille();
      fortsett = false;
    }
    if(millis()> stilleMillis + 500){                       //Skal gjore noe om d er gaatt lang nok tid
      
      fortsett = false;
    }
  }
  
}

void fatalError(int e){                                         //Noe er veldig galt. Det kreves omstart eller sjekk av SD-kort.
//skriv("fatalError", 0);
  //skriv("farilig feil", 1);
  if(e==1){                                                     //SD-kortet er ikke satt opp riktig.
    playTrackNum(11);       
  } else {
    playTrackNum(12);                                           //Noe annet er galt, antagligvis denne koden.
  }
  //skriv("fatalError slutt", 10);
}

void resetMaseTid(){                                            //Paa denne maaten speor systemet brukeren om forklaring eller hjelp er noedvendig til passende tid.
//skriv("resetMaseTid", 0);
  paaTideDur=foersteMaseTid;
  //skriv("resetMaseTid slutt", 10);
}

void siDinTur(){                                                //Foelgende 3 metoder ble opprettet for aa gjoere koden mer oversiktelig, og systemet mer forstaaelig.
//skriv("siDinTur", 0);
  playTrackNum(9);
  //skriv("siDinTur slutt", 10);
}

void siHerStaarDet(){
  //skriv("siHerStarterDet", 0);
  playTrackNum(14);
  //skriv("siHerStaartDet slutt", 10);
}

void siDetteBetyr(){
  //skriv("siDetteBetyr", 0);
  playTrackNum(15);
  //skriv("siDetteBetyr slutt", 10);
}
void snakkFerdig(){
  skriv("snakkFerdig", 0);
  //skriv("snakker", 1);
  while(MP3player.isPlaying()){
    delay(300);
  }
  skriv("snakkFerdig slutt", 10);
}

boolean slaaPaaPgaTilt(){                                       //Returnerer true om bamsen har ligget i ro lenge, og saa blir satt i bevegelse.
////skriv("slaaPaaPgaTilt", 0);
  if(sovTungt){                                                 //Hvis vi har satt i preferanser at bamsen aldri skal gjoere det beskrevet i linjen ovenfor, returnerer vi false.
      ////skriv("slaaPaaPgaTilt (alltid) slutt", 10);
    return false;
  }
  int naatid=millis();
  boolean ventetLenge=naatid>stilleSiden+lenge;                 //Har det gaatt lang nok tid?
  boolean newTiltState=digitalRead(tiltPin);                    //Har det vaert bevegelse?
  //skriv("ventet,tiltstate, saa new",1);
  if(debugPaa){
  //Serial.println(ventetLenge);
  //Serial.println(tiltState);
  //Serial.println(newTiltState);
  }
  if(tiltState!=newTiltState){                                  //dvs. bevegelse.
    //skriv("endring i tilt",1);
    if(ventetLenge){                                            //dvs. ventet lenge.
    ////skriv("slaaPaaPgaTilt (ja) slutt", 10);
    //skriv("ventet lenge",1);
          ////skriv("slaaPaaPgaTilt slutt", 10);
      return true;
    }
    stilleSiden=naatid;
  }
    tiltState=newTiltState;                                     //Oppdater tilt-state slik at vi sammenlikner fra da den ble lagt i ro, ikke da den ble skrudd av.
   
        ////skriv("slaaPaaPgaTilt slutt", 10);
  return false;
}

byte overordnetInnrykk=0;                                       //variabel til skriv(); metoden

void skriv(String info, byte tabs){                             //Egen utskrift til seriell funksjon. Skru av(settdebugPaa til false) under brukertesting for bedre ytelse.
  if(tabs==0){           
    overordnetInnrykk++;
    tabs=overordnetInnrykk;
  } else if (tabs==10){                                         //Slutten av en metode.
    tabs=overordnetInnrykk;  
    overordnetInnrykk--;
  } else {                                                      //Hvis ikke er tabs==1, og det er info om metoden som skal skrives ut.
    tabs=overordnetInnrykk+1; 
  }
  if(debugPaa){
  for(byte b=0;b<tabs;b++){
    Serial.print("\t");
  }
    Serial.println(info);
  }
}


void skrivKommandoVindu(){                                      //Denne metoden er beregnet for evalueringen med wizard og Oz,
                                                               //da vi paa grunn av mulige feil valgte aa gjore det paa denne maaten. for aa redusere antall ting som kunne gaa galt.
  Serial.println("\n Kommandoer:"); 
  Serial.println(" 1 - Hjelp");
  Serial.println(" 2 - Ferdig");
  Serial.println(" 3 - Boknavn (velge valgt bok)");
  Serial.println(" 4 - Oversett");
  Serial.println(" 5 - Natta");
  Serial.println(" 6 - Hei");
  Serial.println("7  Skru av/paa  (ved hjelp av labben) \n");
 }

void tomKoo(){
                                 //Mens systemet faar input fra VRM.
  skriv("tomKoo startet",0);
  while(Serial.available()){       
    skriv("fjernet kommando fra ko",1);
    Serial.read();
  }
 skriv("tomKoo slutt",10);
}
