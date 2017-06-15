#include "mraa/aio.h"
#include "mraa/gpio.h"
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <time.h>
#include <sys/poll.h> 
#include <fcntl.h> 
#include <string.h>
#include <ctype.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <errno.h> 
#include <sys/wait.h>
#include <strings.h> 
#include <openssl/ssl.h> 

/* Initializing OpenSSL */
 
#define h_addr h_addr_list[0] /* for backward compatibility */

/* Initializing OpenSSL */

SSL *sslClient = NULL;

char HOST_NAME[256] = "lever.cs.ucla.edu";
int filedes, portno, logFlag; // log flag is -1 if no shell option and becomes filedes for file if -l is true. 
struct sockaddr_in serv_addr;
struct hostent * server;

int period=1;
char *scale;
char timeString[9];  // space for "HH:MM:SS\0"
time_t current_time;
struct tm * time_info;
int exit_flag = 0;
time_t record_time;
mraa_gpio_context adc_d3;
int log_flag=0; 
char * buffer; 
int count = 0;
int start_flag=1;
char myID[256] = "504646937"; 
void loop();



void setPortNumber(int argc, char ** argv)
{
	for ( int i = 0; i < strlen(argv[argc-1]);i++)
	{
		if(!isdigit(argv[argc-1][i]))
			{
				fprintf(stderr,"Arguement Error\n") ;
				exit(1);
			}
	}

	portno = atoi(argv[argc-1]);	
}

void socket_handler()
{
		filedes = socket(AF_INET, SOCK_STREAM, 0);	//Attempting to read from socket and create an entry in filedes table. 

		if (filedes < 0) 						
		{
			fprintf(stderr, "Failed to open socket: %s\n", strerror(errno));
			exit(1);
		}

		server = gethostbyname(HOST_NAME);			//Get server information.

		if (server == NULL)
		{
			fprintf(stderr, "Host not Found: %s\n", strerror(errno));
			exit(1);
		}

		bzero((char *) &serv_addr, sizeof(serv_addr)); // zero out the bytes contating the variable serv_addr 
		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length); //copy address of the server into the server address field in serv_addr. 
		serv_addr.sin_port = htons(portno); //Hook them up to the correct portno. 

		if (connect(filedes,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		{
			fprintf(stderr, "Error connecting to server: %s\n", strerror(errno));
			exit(1); 
		}
		
		char id[256]; 
		sprintf(id,"ID=%s\n",myID);	
			write(filedes, id, strlen(id));
}

void processInput()
{

	
	char Buffer[256]; 

	int size ; 

		size = read(filedes,&Buffer,256);

	for(int incr = 0 ; incr < size; incr++)
	{
		char buff = Buffer[incr]; 

		if(buff == '\n') 
		{


			buffer[count] = '\0';

			if(buffer[0] == 'P')
			{
				char * PERIOD = "PERIOD=";

				int i = 0;
				for(i =0 ; i < strlen(PERIOD);i++)
				{
					if(PERIOD[i] == buffer[i])
						continue; 

					else 
					{	fprintf(stderr,"INVALID ARGUMENT"); 
						exit(1); 
					}
				
				}


				char* arg = malloc(16); 
				int index = 0;

				for(int j = i;j < count;j++)
				{
					arg[index] = buffer[j];
					index++;
				}
				
				arg[index] = '\0';
				for(int i=0; i < strlen(arg);i++)
				{
					if(!isdigit(arg[i]))
					{
						fprintf(stderr,"Incorrect argument provided to period\n");
						exit(1);
					}
				}
				period = atoi(arg); 
				count = 0;
			//	return; 
			
			}
			else 
			if(strcmp("OFF",buffer) == 0)	
			{
				char * termi = "OFF\n";
				if(log_flag > 0)
					write(log_flag,termi, strlen(termi));
				exit_flag = 1; 
				loop();
				exit(0);
			}
			else
			if(strcmp("SCALE=F",buffer)==0)
			{
				scale="f";
				count=0;
			//	return;
			}
			else
			if(strcmp("SCALE=C",buffer)==0)
			{
				scale="c";
				count=0;
				//return;
			}
			else
			if(strcmp("STOP",buffer)==0)
			{
				start_flag = 0;
				count=0;
			//	return;
			}
			else
			if(strcmp("START",buffer)==0)
			{
				start_flag = 1;
				count=0;
			//	return;
			}
			else{

			count=0;
			char* log_message = "Incorrect option or argument provided\n";
		        write(log_flag,log_message,strlen(log_message)); 	
			exit(1); 
			}

			
			if(log_flag > 0)
			{
				write(log_flag,buffer,strlen(buffer));
				char * newline = "\n";
				write(log_flag,newline,1);
			
			}
		}

		else 
		{
			buffer[count] = buff; 
			count++;
		}
					
	}
}


void getOptHandler(int argc, char ** argv)
{

  static struct option long_opts[] =
  {
	{"period",required_argument, 0, 'p'},
	{"scale",required_argument, 0, 's'},
	{"log",required_argument,0,'l'},
	{"id", required_argument, 0, 'i'},
	{"host", required_argument, 0, 'h'},
	{0,0,0,0}
  };

int opt = 0; 
scale = malloc(4); 

while( (opt = getopt_long(argc, argv, "p:s:l:i:h:", long_opts, NULL)) != -1)
  {
	  switch(opt)
	  {
		 case 'i':
			if(atoi(optarg) < 100000000 || atoi(optarg) > 999999999)
			{
				fprintf(stderr, "Invalid ID\n");
				exit(1);
			}
			strcpy(myID, optarg);
			break;
		 case 'h': 
			strcpy(HOST_NAME, optarg);
			break;	
		 case 'l': 
		        log_flag = creat(optarg,0666) ;
			break;
		 case 'p':
		       if(!isdigit(optarg[0]))
		       	{
				fprintf(stderr,"Bad argument to period\n");
				exit(1);
			}		
			period = atoi(optarg);
			break;
		case 's': 
			if(optarg[0] != 'C' && optarg[0] != 'F')
			{
				fprintf(stderr,"Incorrect scale argument\n");
				exit(1);
			}

			if(optarg[0] == 'C')
				scale = "c\n"; 
			else 
				scale = "f\n";	
			break;	
		default: 
			fprintf(stderr,"Optarg Failiure"); 
			exit(1); 
	  }
  }

}


void loop()
{

	time(&current_time);
	time_info = localtime(&current_time);
	if(!exit_flag)
		if(difftime(current_time, record_time) < period)
			return; 

		if(!start_flag && !exit_flag)
			return; 


	strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);

    mraa_aio_context adc_a0;
    uint16_t adc_value = 0;
    float adc_value_float = 0.0;
    adc_a0 = mraa_aio_init(0);
    if (adc_a0 == NULL)
    {
        return;
    }

    int R0 = 100000;
    int B = 4275; 

 
        adc_value = mraa_aio_read(adc_a0);
	float R = 1023.0/adc_value-1.0;

	R = R0*R;
	float temperatureC = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet
	float temperatureF = ((temperatureC * (1.8)) + 32) ;      
 	double temp; 

	if(scale[0] == 'c')
		temp = (double) temperatureC; 
	else 
		temp = (double) temperatureF;

	if(!exit_flag)
	{
		char *dataVal = malloc(20);
		sprintf(dataVal, "%s %0.1f\n", timeString, temp);
		//fprintf(stdout, "%s",dataVal);

			write(filedes,dataVal,strlen(dataVal));

		if( log_flag > 0)
			write(log_flag, dataVal, strlen(dataVal));
	}

	else

	{
		char * dataVal = malloc(20);
		sprintf(dataVal, "%s %s\n",timeString,"SHUTDOWN");
		//fprintf(stdout, "%s", dataVal); 
			write(filedes, dataVal, strlen(dataVal)); 	

		if(log_flag > 0)
			write(log_flag, dataVal, strlen(dataVal));
		return; 
	}
	
	mraa_aio_close(adc_a0);

	time(&record_time);
//    return MRAA_SUCCESS;
}




int main(int argc, char** argv)
{

	getOptHandler(argc,argv);
	setPortNumber(argc, argv);
	
	
	setPortNumber(argc,argv);
	socket_handler(); 

	struct pollfd pollen[1];
	
	pollen[0].fd = filedes;
	pollen[0].events = POLLIN | POLLERR | POLLHUP;

	buffer = malloc(256); 

    	adc_d3 = mraa_gpio_init(3);

	mraa_gpio_dir(adc_d3,MRAA_GPIO_IN);

	
	loop(); 
	time(&record_time); 


	while(1) 
	{	
	    poll(pollen,1,0); 
	
	    if(pollen[0].revents & POLLIN) 
	    {
		    processInput(); 
	    }
	
	    if(mraa_gpio_read(adc_d3))
		{
			exit_flag = 1;
			loop();
			exit(0);
		}
			

			loop();

	}
	

	mraa_gpio_close(adc_d3);

}
