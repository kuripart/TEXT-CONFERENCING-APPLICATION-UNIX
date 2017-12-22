#ifndef CONFERENCE_HANDLER_H
#define CONFERENCE_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// you're the best!!!
void read_line_from_file(char* file_name, char* out_line){

    size_t length;

    FILE *session_file= fopen(file_name, "r");
    fseek (session_file, 0, SEEK_SET);
    getline(&out_line, &length, session_file);
    fclose(session_file);
}


// Function helps in creating a new session for every client

void create_new_conf_sess(char *session_name_temp, char *client_id){

    // creating a UNIVERSAL session file
    FILE *session_file = fopen("session_file.txt", "a"); 
    fseek(session_file , 0L, SEEK_END);
    char *txt = ".txt";

    // creating individual session files

    char *session_file_txt = malloc(strlen(session_name_temp)+strlen(txt)+1);//+1 for the null-terminator
    strcpy(session_file_txt, session_name_temp);
    strcat(session_file_txt, txt);


    FILE *session_file_single = fopen(session_file_txt, "w");
    fclose(session_file_single);
    fprintf(session_file, "%s:", session_name_temp);

    // creating client file
    char *client_text = ".txt";
    char *client_file_txt = malloc(strlen(client_id)+strlen(client_text)+1);//+1 for the null-terminator
    strcpy(client_file_txt, client_id);
    strcat(client_file_txt, client_text);
   
    FILE *client_file_single = fopen(client_file_txt, "a");
    fclose(client_file_single);

    fclose(session_file);

    add_file_name(session_file_txt);
    add_file_name(client_file_txt);
}


//Function helps each client to join sessions

int join_session(char *session_id, char *client_id){

    char *txt = ".txt";
    
    // session.txt file
    char *temp_str = malloc(strlen(session_id)+strlen(txt)+1);//+1 for the null-terminator
    strcpy(temp_str, session_id);
    strcat(temp_str, txt);

    FILE *session_file_to_join = fopen(temp_str, "a");
    fseek(session_file_to_join , 0L, SEEK_END);
    fprintf(session_file_to_join, "%s:", client_id);
    fclose(session_file_to_join);

    // client.txt file
    char *temp_str_client = malloc(strlen(client_id)+strlen(txt)+1);//+1 for the null-terminator
    strcpy(temp_str_client, client_id);
    strcat(temp_str_client, txt);

    FILE *client_file = fopen(temp_str_client, "a");
    if(client_file == NULL){
        client_file = fopen(temp_str_client, "w");
        fclose(client_file);
        add_file_name(temp_str_client);
        client_file = fopen(temp_str_client, "a");
    }
    fseek(client_file , 0L, SEEK_END);
    fprintf(client_file, "%s:", session_id);
    fclose(client_file);

    return 0;
}


//Helper function to delete a entries in a file

void delete_line_entry(char *file_line, char *str_to_delete, char *return_str){

    int str_length = strlen(file_line);
    char curr_char;
    char curr_substr[BUF_SIZE];
    memset(curr_substr,0,sizeof(curr_substr));
    int substr_index = 0;

    for(int i = 0; i < str_length; i++){
        curr_char = file_line[i];
        if (curr_char == ':'){
            //Evaluate substring

            if (strcmp(curr_substr,str_to_delete) == 0){
                //Substring holds the stuff to delete ... should be ignored!                
            }
            else{
                //Check if return_str is empty ...
                curr_substr[substr_index] = curr_char;
                if (strlen(return_str) == 0){
                    strcpy(return_str,curr_substr);
                }
                else{
                    strcat(return_str,curr_substr);
                }
            }

            //RESET substring every time a ':' is reached ...
            memset(curr_substr,0,sizeof(curr_substr));
            substr_index = 0;
        }
        else{
            curr_substr[substr_index] = curr_char;
            substr_index += 1;
        }
    }
}



//Function to leave a session the client is taking part in

void leave_session(char *session_id, char *client_id){

    char *txt = ".txt";
    size_t length = 0;
    char return_str[BUF_SIZE];
    char modified_session_line[BUF_SIZE];
    char modified_client_line[BUF_SIZE];
    char *original_session_line;
    char *original_client_line;
    memset(return_str, 0, sizeof(return_str));


    //yayy!!
    memset(modified_session_line, 0, sizeof(modified_session_line));
    memset(modified_client_line, 0, sizeof(modified_client_line));

    ////////////OPENING FILES!!!////////////////////
    char *temp_str = malloc(strlen(session_id)+strlen(txt)+1);//+1 for the null-terminator
    strcpy(temp_str, session_id);
    strcat(temp_str, txt);

    FILE *session_file = fopen(temp_str, "r");
    getline(&original_session_line, &length, session_file);
    fclose(session_file);


    // client.txt file
    char *temp_str_client = malloc(strlen(client_id)+strlen(txt)+1);//+1 for the null-terminator
    strcpy(temp_str_client, client_id);
    strcat(temp_str_client, txt);

    FILE *client_file = fopen(temp_str_client, "r");
    getline(&original_client_line, &length, client_file);
    fclose(client_file);

    //Read entire line of Session File into string
    delete_line_entry(original_session_line, client_id, modified_session_line);
    int ret;
    if(strlen(modified_session_line) == 0){
        ret = remove(temp_str);
        if(ret == 0){
            printf("Session File %s deleted sucessfully\n", session_id);
            remove_file_name(temp_str);
        }else{
            printf("Session File %s NOT deleted\n", session_id);
        }
    }else{
        session_file = fopen(temp_str, "w");
        fprintf(session_file, "%s", modified_session_line);
        fclose(session_file);
    }

    //Read entire line of Client File into string
    delete_line_entry(original_client_line, session_id, modified_client_line);

    client_file = fopen(temp_str_client, "w");
    fprintf(client_file, "%s", modified_client_line);
    fclose(client_file);

    return;
}


// Function to list sessions currently in the database

void list_sessions(){   
    char *line;
    size_t length = 0;
    int size;
    int size_reach = 0;

    FILE *session_file = fopen("session_file.txt", "r"); //open to read
    fseek(session_file , 0L, SEEK_END); 
    size = ftell(session_file);
    fseek(session_file , 0L, SEEK_SET);
    if (session_file == NULL)
        exit(1);

    printf("The active sessions are as follows: \n");

    getline(&line, &length, session_file);

    char* packet_str = line;
    char* line_session = (char*)malloc(100 * sizeof(char));
    char* line_pwd = (char*)malloc(100 * sizeof(char));
    
    //Separate line into strings ...
    int start = 0;
    //Read value till first colon into total_frag
    int index = 0;
    int index_line_session = 0;

    int first_itr = 0;
    while(size_reach < size){
        if(packet_str[start + index] == ':'){
            printf("%s\n\n", line_session);
            first_itr++;
            index_line_session = 0;
            for(int i = 0; i < strlen(line_session); i++){
                line_session[i] = ' ';
            }
        }
        if(first_itr == 0){
            line_session[index_line_session] = packet_str[start + index];
            index++;
            index_line_session++;
            size_reach++;
        }else{
            line_session[index_line_session] = packet_str[start + index + 1];
            index++;
            index_line_session++;
            size_reach++;
        }
    }
}



// Function to list clients in a session


void list_session_clients(char *session_id){

    char *line;
    size_t length = 0;
    int size;
    int size_reach = 0;

    char *txt = ".txt";
    char *temp_str = malloc(strlen(session_id)+strlen(txt)+1);//+1 for the null-terminator
    strcpy(temp_str, session_id);
    strcat(temp_str, txt);

    FILE * session_file = fopen(temp_str, "r"); //open to read
    fseek(session_file , 0L, SEEK_END); 
    size = ftell(session_file);
    fseek(session_file , 0L, SEEK_SET);
    if (session_file == NULL)
        exit(1);

    printf("The active sessions are as follows: \n");
    
    getline(&line, &length, session_file);

    char* packet_str = line;
    char* line_session = (char*)malloc(100 * sizeof(char));
    char* line_pwd = (char*)malloc(100 * sizeof(char));
    
    //Separate line into strings ...
    int start = 0;
    //Read value till first colon into total_frag
    int index = 0;
    int index_line_session = 0;

    int first_itr = 0;
    while(size_reach < size){
        if(packet_str[start + index] == ':'){
            printf("%s\n\n", line_session);
            first_itr++;
            index_line_session = 0;
            for(int i = 0; i < strlen(line_session); i++){
                line_session[i] = ' ';
            }
        }
        if(first_itr == 0){
            line_session[index_line_session] = packet_str[start + index];
            index++;
            index_line_session++;
            size_reach++;
        }else{
            line_session[index_line_session] = packet_str[start + index + 1];
            index++;
            index_line_session++;
            size_reach++;
        }
    }

}


//Helper function to search for a substring in a line

int search_line_field(char *file_line, char *str_to_find){

    int str_length = strlen(file_line);
    char curr_char;
    char curr_substr[BUF_SIZE];
    memset(curr_substr,0,sizeof(curr_substr));
    int substr_index = 0;

    for(int i = 0; i < str_length; i++){

        curr_char = file_line[i];

        //NOTE: constant delimiter field for this function => ':' !!!!!!!!!!
        if (curr_char == ':'){

            //Evaluate substring
            if (strcmp(curr_substr,str_to_find) == 0){
                //If substring == the str_to_find, return 1 ...
                return 1;              
            }
            else{
                //DO NOTHING!
            }

            //RESET substring every time a ':' is reached (should only get here
            // when the substring you find is not the one you're looking for ...)
            memset(curr_substr,0,sizeof(curr_substr));
            substr_index = 0;
        }
        else{
            curr_substr[substr_index] = curr_char;
            substr_index += 1;
        }

    }
    return 0;
}


//Helper function for transitioining to sessions during multiple conferencing

void transition_to_session(char *new_session_name, char *client_name){

    // current_session.txt file
    char *active_txt = "_active_session.txt";
    char *active_txt_str = malloc(strlen(client_name)+strlen(active_txt)+1);//+1 for the null-terminator
    strcpy(active_txt_str, client_name);
    strcat(active_txt_str, active_txt);

    FILE *client_active_session_file = fopen(active_txt_str, "w");
    fprintf(client_active_session_file, "%s", new_session_name);
    fclose(client_active_session_file);
}


// Swap session during multiple conferencing

void swap_session(char *new_session_name, char *client_name){
    //new_session_name --> the session you want to change to

    char store[BUF_SIZE];
    strcpy(store,new_session_name);
    size_t length;

    // client.txt file
    char *txt = ".txt";
    char *temp_str_client = malloc(strlen(client_name)+strlen(txt)+1);//+1 for the null-terminator
    strcpy(temp_str_client, client_name);
    strcat(temp_str_client, txt);

    char out_line[BUF_SIZE];
    read_line_from_file(temp_str_client, out_line);

    //FUNCTION returns a 1 if FOUND, 0 if NOT FOUND
    int session_exists = search_line_field(out_line,store);

    if(session_exists){
        transition_to_session(store,client_name);     
    }
    else{
        printf("SESSION TO SWAP TO, IS NOT VALID!!!!\n");
    }
}


// Function to display current session of the client

void find_current_session(char *client_name, char *current_session){

    //printf("ENTERING FUNCTION: find_current_session%s\n");

    char *txt = "_active_session.txt";
    char *line;
    size_t length=  0;
    char *temp_str_client = malloc(strlen(client_name)+strlen(txt)+1);//+1 for the null-terminator
    strcpy(temp_str_client, client_name);
    strcat(temp_str_client, txt);
    //printf("ENTERING FILE: %s\n",temp_str_client);

    FILE *client_file = fopen(temp_str_client, "r");
    getline(&line, &length, client_file);
    fclose(client_file);
    strcpy(current_session, line);

}


// Initialise client list

void init_clients_list(char list_of_clients[MAX_CLIENTS][BUF_SIZE]){

  for(int i = 0; i<MAX_CLIENTS; i++){
    memset(list_of_clients[i],0,BUF_SIZE);
  }
}

// Function to get clients in a session

int acquire_fellow_clients(char *exclude_client_name, char *client_current_session, char other_clients[MAX_CLIENTS][BUF_SIZE]){

    //CAN'T USE char** to refer to the array, if you want to strcpy values back into this array of strings ... -_-

    char *txt = ".txt";
    char *line;
    size_t length;
    
    // session.txt file
    char *temp_str = malloc(strlen(client_current_session)+strlen(txt)+1);//+1 for the null-terminator
    strcpy(temp_str, client_current_session);
    strcat(temp_str, txt); 

    char file_line[BUF_SIZE];
    read_line_from_file(temp_str,file_line); 

    char line_buf[BUF_SIZE];
    strcpy(line_buf,file_line);
    int str_length = strlen(line_buf);
    char curr_char;
    char curr_substr[BUF_SIZE];
    memset(curr_substr,0,sizeof(curr_substr));
    int substr_index = 0;
    int num_elems_in_other_clients = 0;

    //Make a loop that iterates through every character in line
    for(int i = 0; i < str_length; i++){

        curr_char = line_buf[i];
        //NOTE: constant delimiter field for this function => ':' !!!!!!!!!!

        if (curr_char == ':'){
            //Evaluate substring

            if (strcmp(curr_substr,exclude_client_name) == 0){
                //Don't acquire into array of OTHER CLIENTS <do nothing>       
            }
            else{
                //Acquire into array of OTHER CLIENTS
                char copy_temp_str[BUF_SIZE];
                strcpy(copy_temp_str,curr_substr);
                strcpy(other_clients[num_elems_in_other_clients],copy_temp_str);
                num_elems_in_other_clients += 1;
            }

            //RESET substring every time a ':' is reached (should only get here
            // when the substring you find is not the one you're looking for ...)
            memset(curr_substr,0,sizeof(curr_substr));
            substr_index = 0;

        }
        else{
            curr_substr[substr_index] = curr_char;
            substr_index += 1;
        }
    }
    return num_elems_in_other_clients;
}

// THE FUNCTIONS BELOW ARE NOT BEING USED FOR THE CURRENT DESIGN

void acquire_index_info(char *client_id, int field_no, char *return_str){
    //field # 1 ---> socket number
    //field # 2 ---> session id

    size_t len;
    char line[256];
    int temp_len;
    char curr_char;
    char *txt = "_index.txt";
    char *temp_str_client = malloc(strlen(client_id)+strlen(txt)+1);//+1 for the null-terminator
    strcpy(temp_str_client, client_id);
    strcat(temp_str_client, txt);
    int semi_counter = 0;
    int buff_char_count = 0;
       
    memset(line, 0, strlen(line));
    FILE * client_file = fopen(temp_str_client, "r");
    fseek (client_file, 0, SEEK_END);
    len = (size_t) ftell(client_file);
    fseek (client_file, 0, SEEK_SET);
    fgets(line, sizeof(line), client_file);
    fclose(client_file);

    for(int i = 0; i < len; i++){
        curr_char = line[i];

        if(curr_char == ':'){
            semi_counter++; 
            if(semi_counter ==  field_no){
                return;
            }else{
                memset(return_str, 0, strlen(return_str));
                buff_char_count = 0;
            }

        }else{
            return_str[buff_char_count] = line[i];
            buff_char_count++;
        }
    }
    memset(return_str, 0, sizeof(return_str));
}

void acquire_session_info(char *client_id, int field_no, char *return_str){
    //field # 1 ---> socket number
    //field # 2 ---> session id

    size_t len;
    char line[256];
    int temp_len;
    char curr_char;
    char *txt = ".txt";
    char *temp_str_client = malloc(strlen(client_id)+strlen(txt)+1);//+1 for the null-terminator
    strcpy(temp_str_client, client_id);
    strcat(temp_str_client, txt);
    int semi_counter = 0;
    int buff_char_count = 0;
       
    memset(line, 0, strlen(line));
    FILE * client_file = fopen(temp_str_client, "r");
    fseek (client_file, 0, SEEK_END);
    len = (size_t) ftell(client_file);
    fseek (client_file, 0, SEEK_SET);
    fgets(line, sizeof(line), client_file);
    fclose(client_file);

    for(int i = 0; i < len; i++){
        curr_char = line[i];

        if(curr_char == ':'){
            semi_counter++; 
            if(semi_counter ==  field_no){
                return;
            }else{
                memset(return_str, 0, strlen(return_str));
                buff_char_count = 0;
            }

        }else{
            return_str[buff_char_count] = line[i];
            buff_char_count++;
        }
    }

    memset(return_str, 0, sizeof(return_str));

}






#endif /* CONFERENCE_HANDLER_H */