
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<time.h>
#include "p3170086-p3170089-res2.h"

int* numOfSeats;

int CUSTOMERS;
struct timespec start,stop;

int* numOfEmployees;

long* custs;

int* zonea;

int* zoneb;

int* zonec;

int* seatsPerZone;
int employees=9;

int clerks=5;

int tid=0;

int anamoni=0;

int seats=0;	

int income=0;

int total_waiting_time=0;

int total_service_time=0;

int Azone[5][10];

int Bzone[10][10];

int Czone[10][10];

unsigned int seed=0;

int percentzonefull=0;

int percentnoconsec=0;

int percentnopay=0;

int percentpaid=0;


pthread_mutex_t operators;
pthread_mutex_t service_time;
pthread_mutex_t waiting_time_mutex;
pthread_mutex_t employeeAssignment;
pthread_mutex_t transaction;
pthread_mutex_t main_mutex;
pthread_mutex_t bank_account;
pthread_mutex_t keep_seats;
pthread_mutex_t decreaseEmployees;
pthread_mutex_t cashiers;
pthread_mutex_t decreaseCashiers;
pthread_cond_t cond;
pthread_cond_t payment;



int payable(int * seed){
	int random_number=rand_r(seed)%100;
	if(random_number < possibility) return 1;
	return 0;
}

int zoneprobability(int * seed){
	int random_number=rand_r(seed)%100;
	if(random_number < pzonea){
		return 1;
	}else if(random_number >=pzonea && random_number<pzonea+pzoneb){
		return 2;
	}else if (random_number>=pzonea+pzoneb){
		return 3;
	}
	return 0;
}

int findSeats(int nseat,int rows ,int seats,int zone[rows][nseat],int cust,int zonenum){
	int i,j,k,p,n,c;
	int tempres[5];
	int flag=-1;
	int temprow;
	k=0;	
	temprow=-1;
	for(c=0;c<5;c++){tempres[c]=-1;}
	for(i=0;i<rows;i++){
		
		for(j=0;j<nseat;j++){
			if(zone[i][j]==-1){
				tempres[k]=j;
				k=k+1;
				temprow=i;
			}else{
				k=0;
				temprow=-1;
				for(c=0;c<5;c++){tempres[c]=-1;}
			}
			if(k==seats){
				flag=1;
				break;
			}
			
		}
		if(flag==1){break;}
		k=0;	
		temprow=-1;
		for(c=0;c<5;c++){tempres[c]=-1;}
	}
	
	if(flag==1){
		for(p=0; p<5;p++){
			n=tempres[p];
			if(n!=-1){zone[temprow][n]=cust;}
		}
	seatsPerZone[zonenum-1]=seatsPerZone[zonenum-1]-seats;
	}
	
	
	
	return temprow;
}

void update(int argument){
	for(int k=0;k<8;k++){
		if(numOfEmployees[k]==argument){
			numOfEmployees[k]=-1;
			break;
		}
	}

}

void find(int argument){
	for(int k=0;k<8;k++){ 
		if(numOfEmployees[k]==-1){
			numOfEmployees[k]=(int)argument;
 			break;
		}
	}
}
void * consume(void * arg)
{	
	int random_number=rand_r(&seed);
	int tickets=random_number%seatshigh+seatslow;

	random_number=rand_r(&seed);
	int waiting_time = random_number%(timehigh+1-timelow)+timelow;
	int cash_time = random_number%(cashhigh+1-cashlow)+cashlow;
	
	int flag=0;	
	
	int line=-1;

	int tempprice;

	int isPaid=payable(&seed);
	
	int zonecalc=zoneprobability(&seed);
	
	clock_gettime(CLOCK_REALTIME,&start);
		
	pthread_mutex_lock(&service_time);

	total_service_time+=waiting_time;
	total_service_time+=cash_time;

	pthread_mutex_unlock(&service_time);

	custs[(int)arg]=start.tv_sec;

	
	if(employees>0){
			pthread_mutex_lock(&decreaseEmployees);

			employees--;
			
			pthread_mutex_unlock(&decreaseEmployees);
			
	}

	pthread_mutex_lock(&operators);

			while(employees==0)
				pthread_cond_wait(&cond,&operators);

	pthread_mutex_unlock(&operators);

	
	
	pthread_mutex_lock(&employeeAssignment);	
	
	find((int)arg);
	
	pthread_mutex_unlock(&employeeAssignment);

	sleep(waiting_time);
	

	pthread_mutex_lock(&main_mutex);
		
	
	if(seatsPerZone[zonecalc-1]==0){
		
		printf("\n Zone %i is full!",zonecalc);
		employees++;
		update((int)arg);
		percentzonefull++;
		
	}else if(seatsPerZone[zonecalc-1]-tickets>=0){
		
		if(zonecalc==1){
			
			line=findSeats(nseat,zonearows,tickets,Azone,(int)arg,zonecalc);
			
		}else if(zonecalc==2){
			
			line=findSeats(nseat,zonebrows,tickets,Bzone,(int)arg,zonecalc);
			
			
		}else{
			
			line=findSeats(nseat,zonecrows,tickets,Czone,(int)arg,zonecalc);
			
			
		}

		if (line==-1){
			percentnoconsec++;
			printf("\n Not enough consecutive seats for customer %i in zone %i",(int)arg,zonecalc);
			employees++;
			update((int)arg);
			seatsPerZone[zonecalc-1]=seatsPerZone[zonecalc-1]+tickets;	
		}else{
			
			flag=1;
		
		}
	}
	
	pthread_mutex_unlock(&main_mutex);
	pthread_cond_signal(&cond);
	if(flag==1){
		
		if(clerks>0){
			pthread_mutex_lock(&decreaseCashiers);

			clerks--;
			
			pthread_mutex_unlock(&decreaseCashiers);
			
		}
		
		pthread_mutex_lock(&cashiers);
			while(clerks==0){
				pthread_cond_wait(&payment,&cashiers);
			}

		pthread_mutex_unlock(&cashiers);
		sleep(cash_time);
		pthread_mutex_lock(&bank_account);
		if(isPaid==0){	
	
			seatsPerZone[zonecalc-1]=seatsPerZone[zonecalc-1]+tickets;
			
			if(zonecalc==1){
				for(int j=0;j<nseat;j++){
					if(Azone[line][j]==(int)arg){Azone[line][j]=-1;}
			}
		
			
			}else if(zonecalc==2){
				for(int j=0;j<nseat;j++){
					if(Bzone[line][j]==(int)arg){Bzone[line][j]=-1;}
				}
			}else{
				for(int j=0;j<nseat;j++){
					if(Czone[line][j]==(int)arg){Czone[line][j]=-1;}
				}
			}
	
				 
			printf("\nCredit card declined of customer %i ", (int)arg);
			employees++;
			update((int)arg);
			clerks++;
			percentnopay++;
			
			pthread_cond_signal(&payment);
		}else{
				if(zonecalc==1){
					tempprice=tickets*pricezonea;
					income+=tempprice;
					
				}else if (zonecalc==2){
					tempprice=tickets*pricezoneb;
					income+=tempprice;
					
				}else if (zonecalc==3){
					tempprice=tickets*pricezonec;
					income+=tempprice;
					
				}
			
			
			pthread_cond_signal(&payment);
			clerks++;
		}
		
		
	}
	pthread_mutex_unlock(&bank_account);

	pthread_mutex_lock(&transaction);	
	
	if(flag&&isPaid==1){	
		percentpaid++;
		printf("\nCustomer %i bought %i tickets ,cost : %i at line %i.",          	(int)arg,tickets,tempprice,line);
		flag=0;
		employees++;
		update((int)arg);

	}
	if(zonecalc==1){
					for(int i=0; i<zonearows;i++){
						for(int j=0;j<nseat;j++){
							if(Azone[i][j]==(int)arg){
								printf("\n Seat %i at row %i in zone %i, bought by customer %i ",j,i,zonecalc,(int)arg);	
							}
						}
						
					}
					printf("\n");
				}else if (zonecalc==2){
					for(int i=0; i<zonebrows;i++){
						for(int j=0;j<nseat;j++){
							if(Bzone[i][j]==(int)arg){
								printf("\nSeat %i at row %i in zone %i, bought by customer %i",j,i,zonecalc,(int)arg);	
							}
						}
						
					}
					printf("\n");
				}else if (zonecalc==3){
					for(int i=0; i<zonecrows;i++){
						for(int j=0;j<nseat;j++){
							if(Czone[i][j]==(int)arg){
								printf("\nSeat %i at row %i in zone %i, bought by customer %i ",j,i,zonecalc,(int)arg);	
							}
						}
						
					}
					printf("\n");
				}
	printf("\nTransaction ID of customer %i is: %i ",(int)arg,tid);
	tid++;
	pthread_mutex_unlock(&transaction);

	
	pthread_mutex_lock(&keep_seats);
	
	printf("\n");
	clock_gettime(CLOCK_REALTIME,&stop);
	
	custs[(int)arg]=stop.tv_sec-custs[(int)arg];
	anamoni=custs[(int)arg]-waiting_time-cash_time;

	pthread_mutex_unlock(&keep_seats);

	pthread_mutex_lock(&waiting_time_mutex);
		total_waiting_time+=anamoni;
	pthread_mutex_unlock(&waiting_time_mutex);
	
	pthread_exit(NULL);
}


int main(int argc, char ** argv)
{	
	int j;
	int i;		//int declared and used for loop purposes 
	int result;	//int declared and used to store possible failure in execution 	of mutex initiation or destroyance tasks
	void * t_return;

	if(argc!=3){
		printf("Not enough arguments");	
		return -1;
	}
	//storing the input 
	seed=atoi(argv[2]);	
	CUSTOMERS=atoi(argv[1]);
	
	//creating arrays using memory allocation 
	pthread_t * customer=malloc(CUSTOMERS*sizeof(pthread_t));	
	custs=malloc(CUSTOMERS*sizeof(long));
	numOfEmployees=malloc(NumberOfEmployees*sizeof(int));
	numOfSeats=malloc(GlobalNumberOfSeats*sizeof(int));
	seatsPerZone=malloc(3*sizeof(int));
	seats=GlobalNumberOfSeats;

	seatsPerZone[0]=zonearows*nseat;
	seatsPerZone[1]=zonebrows*nseat;
	seatsPerZone[2]=zonecrows*nseat;


	//mutex initiation 
	result=pthread_mutex_init(&main_mutex,NULL);

	if(result!=0)
	{
		perror("\nMutex initialization failed");
		exit(EXIT_FAILURE);
	}
	
	result=pthread_mutex_init(&waiting_time_mutex,NULL);

	if(result!=0)
	{
		perror("\nMutex initialization failed");
		exit(EXIT_FAILURE);
	}
	
	result=pthread_mutex_init(&employeeAssignment,NULL);

	if(result!=0)
	{
		perror("\nMutex initialization failed");
		exit(EXIT_FAILURE);
	}
	
	result=pthread_mutex_init(&service_time,NULL);

	if(result!=0)
	{
		perror("\nMutex initialization failed");
		exit(EXIT_FAILURE);
	}

	
	
	result=pthread_mutex_init(&decreaseEmployees,NULL);

	if(result!=0)
	{
		perror("\nMutex initialization failed");
		exit(EXIT_FAILURE);
	}
	
	
	result=pthread_mutex_init(&bank_account,NULL);

	if(result!=0)
	{
		perror("\nMutex initialization failed");
		exit(EXIT_FAILURE);
	}

	result=pthread_mutex_init(&transaction,NULL);

	if(result!=0)
	{
		perror("\nMutex initialization failed");
		exit(EXIT_FAILURE);
	}	
	
	result=pthread_mutex_init(&keep_seats,NULL);

	if(result!=0)
	{
		perror("\nMutex initialization failed");
		exit(EXIT_FAILURE);
	}

	result=pthread_mutex_init(&decreaseCashiers,NULL);

	if(result!=0)
	{
		perror("\nMutex initialization failed");
		exit(EXIT_FAILURE);
	}

	result=pthread_mutex_init(&cashiers,NULL);

	if(result!=0)
	{
		perror("\nMutex initialization failed");
		exit(EXIT_FAILURE);
	}

	result=pthread_mutex_init(&operators,NULL);

	if(result!=0)
	{
		perror("\nMutex initialization failed");
		exit(EXIT_FAILURE);
	}
	
	result=pthread_cond_init(&cond,NULL);

	if(result!=0)
	{
		perror("\nCondition variable initialization failed");
		exit(EXIT_FAILURE);
	}

	


	result=pthread_cond_init(&payment,NULL);

	if(result!=0)
	{
		perror("\nCondition variable initialization failed");
		exit(EXIT_FAILURE);
	}


	for(i=0;i<GlobalNumberOfSeats;i++)  numOfSeats[i]=-1;
	
	for(i=0; i<NumberOfEmployees;i++) numOfEmployees[i]=-1;

	for(i=0; i<zonearows;i++){
		for(j=0;j<nseat;j++){
			Azone[i][j]=-1;
		}
	}

	for(i=0; i<zonebrows;i++){
		for(j=0;j<nseat;j++){
			Bzone[i][j]=-1;
		}
	}

	for(i=0; i<zonecrows;i++){
		for(j=0;j<nseat;j++){
			Czone[i][j]=-1;
		}
	}
	for(i=0;i<CUSTOMERS;i++)
	{
		result=pthread_create(&customer[i],NULL,consume,(void *)i);

		if(result!=0)
		{
			perror("\nThread creation failed");
			exit(EXIT_FAILURE);
		}

		printf("\nCustomer %i ready to consume",i);
	}


	for(i=0;i<CUSTOMERS;i++)
	{
		result=pthread_join(customer[i],&t_return);

		if(result!=0)
		{
			perror("\nThread join failed");
			exit(EXIT_FAILURE);
		}
	} 
	
	//printing final results 
	printf("\nFinal Seat Structure of Zone A\n");
	for(i=0; i<zonearows;i++){
		for(j=0;j<nseat;j++){
			printf(" %i  ",Azone[i][j]);
		}
		printf("\n ");
	}

	printf("\nFinal Seat Structure of Zone B\n");
	for(i=0; i<zonebrows;i++){
		for(j=0;j<nseat;j++){
			printf(" %i  ",Bzone[i][j]);
		}
		printf("\n ");
	}

	printf("\nFinal Seat Structure of Zone C\n");
	for(i=0; i<zonecrows;i++){
		for(j=0;j<nseat;j++){
			printf(" %i  ",Czone[i][j]);
		}
		printf("\n ");
	}


	printf("\nTotal income is: %i ", income);
	printf("\nTotal waiting time is: %i ", total_waiting_time);
	printf("\nTotal service time is: %i ", total_service_time);
	printf("\nAverage waiting time is %f", total_waiting_time*1.0/CUSTOMERS);
	printf("\nAverage service time is %f", total_service_time*1.0/CUSTOMERS);
	printf("\nPercentage of full zone: %f ", percentzonefull*1.0/(CUSTOMERS)*100);
	printf("\nPercentage of not enough consecutive seats in zone: %f ", percentnoconsec*1.0/(CUSTOMERS)*100);
	printf("\nPercentage of credit card declined: %f ", percentnopay*1.0/(CUSTOMERS)*100);
	printf("\nPercentage of credit card accepted: %f ", percentpaid*1.0/(CUSTOMERS)*100);
	
	
	pthread_mutex_destroy(&operators);
	pthread_mutex_destroy(&employeeAssignment);
	pthread_mutex_destroy(&service_time);
	pthread_mutex_destroy(&waiting_time_mutex);
	pthread_mutex_destroy(&transaction);

	pthread_mutex_destroy(&main_mutex);
	pthread_mutex_destroy(&bank_account);
	pthread_mutex_destroy(&keep_seats);
	
	pthread_mutex_destroy(&decreaseEmployees);
	pthread_mutex_destroy(&cashiers);
	pthread_mutex_destroy(&decreaseCashiers);
	
	pthread_cond_destroy(&cond);
	pthread_cond_destroy(&payment);
	
	free(numOfEmployees);
	free(customer);
	printf("\n");
	printf("Press enter to terminate the program...\n");
	getchar();
	return 0;
}
