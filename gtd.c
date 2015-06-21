#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <argp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define CMD_END "\0"

const char *argp_program_version =
  "gtd 1.0";
const char *argp_program_bug_address =
  "<bug@great-things-done.com>";

// Program documentation.
static char doc[] =
  "gtd -- A command line interface for Great Things Done";

// A description of the arguments we accept.
static char args_doc[] = "commands";

// The options we understand.
static struct argp_option options[] = {
  {"verbose",  'v',  0,              0,                    "Produce verbose output" },
  {"port",     'p',  "port_number",  OPTION_ARG_OPTIONAL,  "Port to which to connect"},
  {"addr",     'a',  "addr",         OPTION_ARG_OPTIONAL,  "Address to which to connect"},
  { 0 }
};

// Used by main to communicate with parse_opt. */
struct arguments
{
  char *args[256];
  int verbose;
  int port_number;
  char *addr;
};

// Parse a single option.
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  // Get the input argument from argp_parse, which we know is a pointer to our arguments structure.
  struct arguments *arguments = state->input;

  switch (key) {
    case 'v':
      arguments->verbose = 1;
      break;
    case 'p':
      arguments->port_number = atoi(arg);
      break;
    case 'a':
      arguments->addr = arg;
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num >= 255)
        //Too many arguments.
        argp_usage (state);

      arguments->args[state->arg_num] = arg;
      arguments->args[state->arg_num + 1] = CMD_END;

      break;

    case ARGP_KEY_END:
      if (state->arg_num < 1)
        // Not enough arguments.
        argp_usage (state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }
  
  return 0;
}

// Our argp parser.
static struct argp argp = { options, parse_opt, args_doc, doc };

void error(char *msg)
{
  perror(msg);
  exit(0);
}

char* concat(char* s1, char* separator, char* s2) {
  char* result = (char*)malloc(strlen(s1) + strlen(separator) + strlen(s2) + 1);

  if (!result) {
    error("Failed to allocate space");
  }

  strcpy(result, s1);
  strcat(result, separator);
  strcat(result, s2);
  return result;
}

char* build_command(char** commands){
  char* result = "";
  int i = 0;

  while (strcmp(CMD_END, commands[i])) {
    result = concat(result, " ", commands[i]);
    i++;
  }

  return result;
}

int main(int argc, char **argv)
{
  struct arguments arguments;
  int sockfd, n;

  struct sockaddr_in serv_addr;
  struct hostent *server;

  char buffer[1028];
  int result_size = 1028;
  char* result;
  char* command;
// Default values.
  arguments.verbose = 0;
  arguments.port_number = 5002;
  arguments.addr = "localhost";

// Parse our arguments; every option seen by parse_opt will be reflected in arguments.
  argp_parse (&argp, argc, argv, 0, 0, &arguments);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    error("ERROR opening socket");
  }

  server = gethostbyname(arguments.addr);

  if (server == NULL) {
    error("ERROR, no such host");
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;

  bcopy((char *)server->h_addr, 
       (char *)&serv_addr.sin_addr.s_addr,
       server->h_length);

  serv_addr.sin_port = htons(arguments.port_number);
  if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
    error("ERROR connecting");
  }

  command = build_command(arguments.args);

  if (write(sockfd, command, strlen(command)) < 0) {
    error("ERROR writing to socket");
  }

  result = "";

  while ((n = read(sockfd, buffer, 1027))) {
    buffer[n] = '\0';
    result = concat(result, "", buffer);
  }

  printf("%s\n", result);

  if (command) {
    free(command);
  }

  if (result) {
    free(result);
  }
  return 0;
}