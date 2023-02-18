#include <pthread.h> 		
#include <time.h> 			
#include <unistd.h> 			
#include <semaphore.h> 		
#include <stdlib.h>			
#include <stdio.h> 			

pthread_t *Studenti;		
pthread_t TA;				

int ConteggioSedie = 0;
int IndiceCorrente = 0;

sem_t TA_Sleep;
sem_t Student_Sem;
sem_t SedieSem[3];
pthread_mutex_t ChairAccess;

// Funzioni dichiarate
void* TA_Attivita(void* arg);
void* Student_Activity(void* threadID);

int main(int argc, char* argv[])
{
    int numero_di_studenti;
    int id;
    srand(time(NULL));

    sem_init(&TA_Sleep, 0, 0);
    sem_init(&Student_Sem, 0, 0);
    for (id = 0; id < 3; ++id)			 
        sem_init(&SedieSem[id], 0, 0);

    pthread_mutex_init(&ChairAccess, NULL);

    if (argc < 2)
    {
        printf("Utilizzo predefinito (5) studenti. \n");
        numero_di_studenti = 5;
    }
    /*lse
   // {
        printf("Numero di studenti specificato. Creazione di %d thread. \n", numero_di_studenti);
        numero_di_studenti = atoi(argv[1]);
    }*/

    Studenti = (pthread_t*)malloc(sizeof(pthread_t) * numero_di_studenti);

    pthread_create(&TA, NULL, TA_Attivita, NULL);
    for (id = 0; id < numero_di_studenti; id++)
        pthread_create(&Studenti[id], NULL, Student_Activity, (void*)(long)id);

    pthread_join(TA, NULL);
    for (id = 0; id < numero_di_studenti; id++)
        pthread_join(Studenti[id], NULL);

    free(Studenti);
    return 0;
}

void* TA_Attivita(void* arg)
{
    printf("~~~~~~~~~~~~~~~~~~~~~~TA sta dormendo.~~~~~~~~~~~~~~~~~~~~~~~\n");
    
    while (1)
    {	
    	sem_wait(&TA_Sleep);
        printf("~~~~~~~~~~~~~~~~~~~~~~TA e' stato svegliato da uno studente.~~~~~~~~~~~~~~~~~~~~~~~\n");
	
        while (1)
        {
            pthread_mutex_lock(&ChairAccess);
            if (ConteggioSedie == 0)
            {
                pthread_mutex_unlock(&ChairAccess);
                printf("~~~~~~~~~~~~~~~~~~~~~~TA si rimette a dormire.~~~~~~~~~~~~~~~~~~~~~~~\n");
                break;
            }

            sem_post(&SedieSem[IndiceCorrente]);
            ConteggioSedie--;
            printf("Lo studente ha lasciato la sedia. Sedie rimanenti %d\n", 3 - ConteggioSedie);
            IndiceCorrente = (IndiceCorrente + 1) % 3;
            pthread_mutex_unlock(&ChairAccess);

            printf("\tTA sta attualmente aiutando lo studente.\n");
            sleep(10);
            sem_post(&Student_Sem);
            usleep(1000);
        }
        break;
    }
    return 0;
}

void* Student_Activity(void* threadID)
{
    int TempoProgrammazione;

    while (1)
    {
        printf("Lo studente %ld sta facendo un compito di programmazione.\n", (long)threadID);
        TempoProgrammazione = rand() % 10 + 1;
        sleep(TempoProgrammazione);

        printf("Lo studente %ld ha bisogno dell'aiuto dell'AT\n", (long)threadID);

        pthread_mutex_lock(&ChairAccess);
        
        int conteggio = ConteggioSedie;
        pthread_mutex_unlock(&ChairAccess);

        if (conteggio < 3) {
            if (conteggio == 0) {
                sem_post(&TA_Sleep);
            } else {
                printf("Lo studente %ld e' seduto su una sedia in attesa che finisca l'AT.\n", (long)threadID);
            }
    
        pthread_mutex_lock(&ChairAccess);
        int index = (IndiceCorrente + ConteggioSedie) % 3;
        ConteggioSedie++;
        printf("Studente seduto sulla sedia. Sedie rimanenti: %d\n", 3 - ConteggioSedie);
        pthread_mutex_unlock(&ChairAccess);
    
        sem_wait(&SedieSem[index]);
        printf("\tLo studente %ld sta ricevendo aiuto dall'AT.\n", (long)threadID);
        sem_wait(&Student_Sem);
        printf("Studente %ld ha lasciato l'aula TA.\n", (long)threadID);
        pthread_exit(threadID);
        } else {
            printf("Lo studente %ld tornerà in un altro momento.\n", (long)threadID);
   	}
   }
}
