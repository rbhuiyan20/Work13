#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>


// struct that stores each piece of data (year, population, boro)
struct pop_entry {
  int year;
  int population;
  char boro[15];
};

// function to help find the size of data file
long size_of_file(char *file) {
    struct stat buffer;
    stat(file, &buffer);
    // returns file size in bytes
    return buffer.st_size;
}

// function to read contents of cvs fle and store piecs of data in structs and then puts this into a new data file
void read_csv(char *csv) {
    int error;
    int file;
    int filesize = size_of_file(csv);
    // array where pop entry  values will be stored
    char data[filesize];

  
    // if file cant open, print the error message
    file = open(csv, O_RDONLY);
    if(file==-1){
        printf("Error: %s\n",strerror(errno));
        return;
    }

   
    // if issue with reading, print the error  message
    error = read(file, data, filesize);
    if(error==-1){
        printf("Error: %s\n",strerror(errno));
        return;
    }

    // to keep track of the #rows of data
    int rows = 0;
    int i;
    for(i=0;i<filesize+1;i++) {
        if (data[i]=='\n'){
          rows++;
        }
    }

    //array of just the borough names
    char *boro[5] = {"Manhattan", "Brooklyn", "Queens", "Bronx", "Staten Island"};
    // struct to collect the entries
    struct pop_entry entries[5*rows-5];
    int index = 0, j;

    // loop to put the csv file into struct 
    for(i=0;i<rows-1;i++){
        int lines[6];
        //
        while(data[index]!='\n'){
            index++;
        }
        index++;
        sscanf(data+index,"%d, %d, %d, %d, %d, %d\n", &lines[0], &lines[1], &lines[2], &lines[3], &lines[4], &lines[5]);
        for(j=1;j<6;j++){
            entries[5*i+j-1].year = lines[0];
            // copies the data
            strcpy(entries[(5*i)+j-1].boro, boro[j-1]);
            entries[(5*i)+j-1].population = lines[j];
        }
    }

    // creates a new data file with permissions
    int dat = open("nyc_pop.data", O_CREAT | O_WRONLY, 0644);
    // if error with opening, print it
    if(dat==-1){
        printf("Error: %s\n",strerror(errno));
        return;
    }
    error = write(dat, entries, sizeof(entries));
    // if error with writing to data file 
    if(dat==-1){
        printf("Error: %s\n",strerror(errno));
        return;
    }
    printf("Reading %s\n", csv);
    printf("Wrote %ld bytes to %s\n",size_of_file("nyc_pop.data"), csv);
}


//Reads Contents of data into array of pop_entry values (array based on size)
void read_data() {
    // simple variable to store the filesize of the new data file
    int filesize = size_of_file("nyc_pop.data");

    // opens up the data file
    int file = open("nyc_pop.data", O_RDONLY);
    if(file==-1){
      //if the file doesnt exist, gotta run -read_csv first
        printf("Problem!: Data file doesn't exist, please run -read_csv\n");
        return;
    }


    // allocates proper memory to teh array based on file size
    struct pop_entry *entries = malloc(file);
    int error = read(file, entries, filesize);
    if(error==-1){
        printf("Error: %s\n",strerror(errno));
        return;
    }

    
    int i;
    int amtData = filesize / sizeof(struct pop_entry);
    // goes through the data array, displays the data 
    for(i=0;i<amtData;i++){
        printf("%d: year:  %d boro: %s pop: %d\n", i, entries[i].year, entries[i].boro, entries[i].population);
    }
    // free memory
    free(entries);
}

// function to ask user for new data to put as a struct value and appends it to the end of data file
void add_data() {
    char input[100];
    struct pop_entry user;
    

    // to take in user input
    printf("Enter year boro pop:\n");
    // get the input
    fgets(input, sizeof(input), stdin);
    if(sscanf(input,"%d %1024[^0-9] %d", &user.year, user.boro, &user.population)!=3) {
      // if input isnt given correctly
        printf("Ente <int> <string> <int> please\n");
        return;
    }

    // if data file cant be opened, it needs to be created
    int file = open("nyc_pop.data", O_WRONLY | O_APPEND);
    if(file==-1){
        printf("Problem!: Data file doesn't exist, please run -read_csv\n");
        return;
    }

    // writes user data into the data file
    int error = write(file, &user, sizeof(user));
    if(error==-1){
        printf("Error: %s\n",strerror(errno));
        return;
    }
}

// prompts user to modify an entry, updates the DATA file
void update_data() {
    struct pop_entry user;
    int amtData = size_of_file("nyc_pop.data") / sizeof(struct pop_entry);
    char input[100];
    read_data();

    // to take in user input fro which entry
    printf("Entry to update: ");
    int index;
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%d", &index);


    // to take in the data u want to update the entry with
    int year, pop;
    char *boro;
    printf("Enter year boro pop:\n");
    fgets(input, sizeof(input), stdin);
    if(sscanf(input,"%d %1024[^0-9] %d", &user.year, user.boro, &user.population)!=3) {
        printf("Please enter it in correct format [int] [string] [int]\n");
        return;
    }

    // opens the data file with read write permissions
    int file = open("nyc_pop.data", O_RDWR);
    if(file==-1){
        printf("Error: \nData file doesn't exist, run -read_csv first.\n");
        return;
    }

    // updates the data the user inputted
    lseek(file, index*sizeof(struct pop_entry), SEEK_SET);
    int error = write(file, &user, sizeof(user));
    if(error==-1){
        printf("Error:\n");
        return;
    }
    close(file);
}

int main(int argc, char *argv[]) {
    if (argv[1]==NULL) {
        printf("Please give a command\n");
        return 0;
        // calls the proper function according to user input
    } else if (strcmp(argv[1],"-read_csv")==0){
        read_csv("nyc_pop.csv");
    } else if (strcmp(argv[1],"-read_data")==0){
        read_data();
    } else if (strcmp(argv[1],"-add_data")==0){
        add_data();
    } else if (strcmp(argv[1],"-update_data")==0){
        update_data();
    } else {
        printf("Please do either -read_csv, -read_data, -add_data, or -update_data\n");
    }
    return 0;
}