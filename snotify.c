#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

int job_finished = 0;

static void skeleton_daemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }

    /* Open the log file */
    openlog ("firstdaemon", LOG_PID, LOG_DAEMON);
}


int is_job_in_queue(char* jobid)
{
    char *cmd = "squeue";

    char buf[128];
    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL)
    {
        printf("Error opening pipe!\n");
        return -1;
    }

    int job_id_seen = 0;

    while (fgets(buf, 128, fp) != NULL)
    {
        // printf("OUTPUT: %s\n", buf);

        if (strstr(buf, jobid) != NULL)
        {
            printf("Job is still in queue!\n");
            job_id_seen = 1;
            return 1;
        }
    }

    if (pclose(fp))
    {
        printf("Command not found or exited with error status\n");
    }

    return 0;
}


int main(int argc, char *argv[])
{
    printf("================== SNOTIFY VERSION 0.1.0 ==================\n");
    
    /* Parse command line arguments */
    char *jobid = NULL;
    char *phone = NULL;
    char *email = NULL;
    char *carrier = NULL;
    int opt;

    while ((opt = getopt(argc, argv, "j:e:p:c:")) != -1)
    {
        switch (opt)
        {
        case 'j':
            jobid = optarg;
            break;
        case 'e':
            email = optarg;
            break;
        case 'p':
            phone = optarg;
            break;
        case 'c':
            carrier = optarg;
            break;
        default:
            break;
        }
    }
    
    if(jobid == NULL) {
    	printf("ERROR: you must specify job id with -j <job-id>!\n");
    	return 1;
    }
    
    if(!is_job_in_queue(jobid)) {
        printf("ERROR: the job id specified is not in squeue\n");
        return 1;
    }
    
    if(phone == NULL && email == NULL) {
    	printf("ERROR: provide email address with -e or phone number and carrier with -p and -c!\n");
    	return 1;
    }
    
    if(phone != NULL && carrier == NULL){
    	printf("ERROR: you must specify your phone carrier with -c (verizon, att, sprint, tmobile, etc)!\n");
    	return 1;
    }
    
    printf("snotify will %s you at %s when job %s is complete!\n", email == NULL ? "text" : "email", email == NULL ? phone : email, jobid);

    /* Start daemon */
    //syslog (LOG_NOTICE, "Daemon started!");
    skeleton_daemon();

    /* Daemon code - keep checking squeue for jobid */
    while(!job_finished)
    {
        if (!is_job_in_queue(jobid))
        {
            printf("JOB COMPLETED!\n");
            job_finished = 1;
            break;
        }
    	
    	sleep(120);
    }
    
    /* Prepare email contents */
    char cmd[256];  						// System command for sending email
    char* to; 							    // Email id that we are sending to
    char subject[256];						// Subject
    char body[512];    					    // Email body
    
    sprintf(subject, "Job %s complete!", jobid);
    
    if(email != NULL)
    {
    	to = email;
    	sprintf(body, "Subject: %s\n\nYour job submission #%s has finished running on Deepthought2!\n\nThis message was sent automatically by snotify", subject, jobid);
    }
    else if (phone != NULL && carrier != NULL)
    {
    	char* domain;
    	
    	if(strcmp(carrier, "verizon") == 0)
    		domain = "vtext.com";
    	else if (strcmp(carrier, "att") == 0)
    		domain = "txt.att.net";
    	else if (strcmp(carrier, "sprint") == 0)
    		domain = "messaging.sprintpcs.com";
    	else if (strcmp(carrier, "tmobile") == 0)
    		domain = "tmomail.net";
    	else if (strcmp(carrier, "boost") == 0)
    		domain = "sms.myboostmobile.com";
    	else if (strcmp(carrier, "cricket") == 0)
    		domain = "mms.cricketwireless.net";
    	else if (strcmp(carrier, "ting") == 0)
    		domain = "message.ting.com";
    	else if (strcmp(carrier, "virgin") == 0)
    		domain = "vmobl.com";
    	else if (strcmp(carrier, "uscellular") == 0)
    		domain = "email.uscc.net";
    	else if (strcmp(carrier, "straighttalk") == 0)
    		domain = "vtext.com";
    	else if (strcmp(carrier, "metropcs") == 0)
    		domain = "mymetropcs.com";
    	else
    	{
	    	printf("ERROR: carrier not recognized\n");
	    	return 1;
    	}
    	
    	char temp[256];
    	sprintf(temp, "%s@%s", phone, domain);
    	
    	to = temp;
    	
    	sprintf(body, "Your job submission #%s has finished running on Deepthought2!\n\nThis message was sent automatically by snotify", jobid);
    }
    
    char tempFile[100];     					        // name of tempfile.

    strcpy(tempFile,tempnam("/tmp","sendmail")); 		// generate temp file name.

    FILE *fp = fopen(tempFile,"w"); 				    // open it for writing.
    fprintf(fp,"%s\n",body);        				    // write body to it.
    fclose(fp);             					        // close it.

    sprintf(cmd,"sendmail %s < %s\n",to,tempFile); 		// prepare command.
    system(cmd);     						            // execute it.
    printf("%s", cmd);

    printf("terminating daemon!\n");
    // syslog (LOG_NOTICE, "Daemon terminated!");
    // closelog();

    return 0;
}
