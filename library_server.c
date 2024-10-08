#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#define PORT 8080

int gettid();

pthread_mutex_t admin_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t books_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t borrowed_books_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t logins_mutex = PTHREAD_MUTEX_INITIALIZER;

int check_admin_credentials(char buffer[])
{
	pthread_mutex_lock(&admin_mutex);
	printf("%d : Acquired lock...\n", gettid()); // For locks
	char buff[100];
	char *token1;
	FILE *admin = fopen("admin_login.txt", "r");
	size_t content = fread(buff, sizeof(char), 100, admin);
	buff[content] = '\0';
	fclose(admin);

	token1 = strtok(buff, ",");
	char *admin_name = token1;
	token1 = strtok(NULL, ",");
	char *admin_pwd = token1;

	char *token2;
	token2 = strtok(buffer, ",");
	token2 = strtok(NULL, ",");
	char *name = token2;
	token2 = strtok(NULL, ",");
	char *pwd = token2;

	sleep(10); // to show locks
	printf("%d : Releasing lock...\n", gettid());
	pthread_mutex_unlock(&admin_mutex); // For locks
	if (!strcmp(admin_name, name) && !strcmp(admin_pwd, pwd))
	{
		return 1; // Credentials matched
	}
	else
	{
		return 0; // Invalid credentials
	}
}

int add_book(char buffer[])
{
	pthread_mutex_lock(&books_mutex);
	char sep[2] = "\n";
	FILE *fd = fopen("books.txt", "a");
	char data[400];

	strcpy(data, buffer + 2);
	strcat(data, sep);

	fseek(fd, 0, SEEK_END);

	int data_written = fwrite(data, sizeof(char), strlen(data), fd);

	fclose(fd);
	pthread_mutex_unlock(&books_mutex);
	if (data_written != 0)
		return 1;
	else
		return 0;
}

int remove_book_by_id(char buffer[])
{
	pthread_mutex_lock(&books_mutex);
	char data[400];
	strcpy(data, buffer + 2);

	FILE *fd = fopen("books.txt", "r");

	char line[400];
	char new_content[1000] = "";
	int found = 0;

	while (fgets(line, sizeof(line), fd) != NULL)
	{
		char *data_to_write = (char *)malloc(400);
		strcpy(data_to_write, line);
		char *bid = strtok(line, ",");
		if (strcmp(bid, data) != 0)
		{
			strcat(new_content, data_to_write);
		}
		else
		{
			found++;
		}
	}
	fclose(fd);

	if (found)
	{
		FILE *fd2 = fopen("books.txt", "w");
		fwrite(new_content, sizeof(char), strlen(new_content), fd2);
		fclose(fd2);
		pthread_mutex_unlock(&books_mutex);
		return 1;
	}
	pthread_mutex_unlock(&books_mutex);
	return 0;
}

void find_book(char buffer[], int new_socket)
{
	pthread_mutex_lock(&books_mutex);
	char failure[50] = "Book not found.";
	char data[400];
	strcpy(data, buffer + 2);
	strtok(data, ",");

	FILE *fd = fopen("books.txt", "r");

	char line[400];
	int found = 0;

	while (fgets(line, sizeof(line), fd) != NULL)
	{
		char *data_to_send = (char *)malloc(400);
		strcpy(data_to_send, line);
		char *first_comma = strchr(line, ','); //stores data before the first occurence of comma
		if (first_comma != NULL)
		{
			*first_comma = '\0';
			if (strcmp(line, data) == 0)
			{
				send(new_socket, data_to_send, strlen(data_to_send), 0);
				found = 1;
				break;
			}
		}
	}

	fclose(fd);

	if (!found)
	{
		send(new_socket, failure, strlen(failure), 0);
	}
	pthread_mutex_unlock(&books_mutex);
}

int update_book_count(char buffer[])
{
	pthread_mutex_lock(&books_mutex);
	char book_id[20];

	char *token = strtok(buffer + 2, ",");
	strcpy(book_id, token);

	char *new_count = strtok(NULL, ",");

	FILE *input_file = fopen("books.txt", "r");
	FILE *output_file = fopen("temp.txt", "w");

	int found = 0;
	char line[400];

	while (fgets(line, sizeof(line), input_file) != NULL)
	{
		char *data = (char *)malloc(400);
		strcpy(data, line);
		char *token2 = strtok(line, ",");
		char current_book_id[20];
		strcpy(current_book_id, token2);

		if (strcmp(current_book_id, book_id) == 0)
		{
			char *name = strtok(NULL, ",");
			char *author = strtok(NULL, ",");
			snprintf(line, sizeof(line), "%s,%s,%s,%s\n", book_id, name, author, new_count);
			found = 1;
			fprintf(output_file, "%s", line);
		}

		else
		{
			fprintf(output_file, "%s", data);
		}
	}

	fclose(input_file);
	fclose(output_file);

	if (rename("temp.txt", "books.txt") != 0)
	{
		perror("Error renaming file");
		pthread_mutex_unlock(&books_mutex);
		return 0;
	}

	pthread_mutex_unlock(&books_mutex);

	return found ? 1 : 0;
}

void display_books(char buffer[], int new_socket)
{
	pthread_mutex_lock(&books_mutex);
	char failure[30] = "No books here!";
	FILE *file = fopen("books.txt", "r");
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	if (file_size == 0)
	{
		send(new_socket, failure, strlen(failure), 0);
		pthread_mutex_unlock(&books_mutex);
		return;
	}
	char *content = (char *)malloc(file_size + 1);
	size_t bytes_read = fread(content, 1, file_size, file);
	content[file_size] = '\0';
	send(new_socket, content, strlen(content), 0);
	pthread_mutex_unlock(&books_mutex);
}

void display_members(char buffer[], int new_socket)
{
	pthread_mutex_lock(&logins_mutex);
	char failure[30] = "No active members yet!";
	FILE *file = fopen("logins.txt", "r");
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	if (file_size == 0)
	{
		send(new_socket, failure, strlen(failure), 0);
		pthread_mutex_unlock(&logins_mutex);
		return;
	}
	char *content = (char *)malloc(file_size + 1);
	size_t bytes_read = fread(content, 1, file_size, file);
	content[file_size] = '\0';
	send(new_socket, content, strlen(content), 0);
	pthread_mutex_unlock(&logins_mutex);
}

int remove_member(char buffer[])
{
	char data[400];
	strcpy(data, buffer + 2);
	pthread_mutex_lock(&logins_mutex);

	FILE *fd = fopen("logins.txt", "r");

	char line[400];
	char new_content[1000] = "";
	int found = 0;
	while (fgets(line, sizeof(line), fd) != NULL)
	{
		char *data_to_write = (char *)malloc(400);
		strcpy(data_to_write, line);
		char *name = strtok(line, ",");
		printf("%s %s", name, data);
		if (strcmp(name, data) != 0)
		{
			strcat(new_content, data_to_write);
		}
		else
		{
			found++;
		}
	}
	fclose(fd);

	if (found)
	{
		FILE *fd2 = fopen("logins.txt", "w");
		fwrite(new_content, sizeof(char), strlen(new_content), fd2);
		fclose(fd2);
		pthread_mutex_unlock(&logins_mutex);
		return 1;
	}
	pthread_mutex_unlock(&logins_mutex);
	return 0;
}

int borrow_book(char buffer[])
{
	pthread_mutex_lock(&books_mutex);
	char book_id[20];
	char sep[2] = "\n";
	char *data_to_write = (char *)malloc(400);
	strcpy(data_to_write, buffer + 2);
	strcat(data_to_write, sep);

	char *token = strtok(buffer + 2, ",");
	strcpy(book_id, token);

	FILE *input_file = fopen("books.txt", "r");
	FILE *output_file = fopen("temp.txt", "w");

	int found = 0;
	char line[400];

	while (fgets(line, sizeof(line), input_file) != NULL)
	{
		char *data = (char *)malloc(400);
		strcpy(data, line);
		char *token2 = strtok(line, ",");
		char current_book_id[20];
		strcpy(current_book_id, token2);
		if (strcmp(current_book_id, book_id) == 0)
		{
			char *name = strtok(NULL, ",");
			char *author = strtok(NULL, ",");
			char *count = strtok(NULL, ",");
			int num = atoi(count);
			if (num > 0)
			{
				num -= 1;
				char new_count[20];
				sprintf(new_count, "%d", num);
				snprintf(line, sizeof(line), "%s,%s,%s,%s\n", book_id, name, author, new_count);
				found = 1;
				fprintf(output_file, "%s", line);
				pthread_mutex_lock(&borrowed_books_mutex);
				FILE *fd = fopen("borrowed_books.txt", "a");
				fseek(fd, 0, SEEK_END);
				fprintf(fd, "%s", data_to_write);
				fclose(fd);
				pthread_mutex_unlock(&borrowed_books_mutex);
			}
			else
			{
				pthread_mutex_unlock(&books_mutex);
				return 0;
			}
		}

		else
		{
			fprintf(output_file, "%s", data);
		}
	}

	fclose(input_file);
	fclose(output_file);

	if (rename("temp.txt", "books.txt") != 0)
	{
		perror("Error renaming file");
		pthread_mutex_unlock(&books_mutex);
		return 0;
	}
	pthread_mutex_unlock(&books_mutex);
	return found ? 1 : 0;
}

int return_book(char buffer[])
{
	pthread_mutex_lock(&borrowed_books_mutex);
	char data[400];
	strcpy(data, buffer + 2);
	char sep[2] = "\n";
	strcat(data, sep);

	FILE *fd = fopen("borrowed_books.txt", "r");

	char line[400];
	char new_content[1000] = "";
	int found = 0;
	while (fgets(line, sizeof(line), fd) != NULL)
	{
		if (strcmp(line, data) != 0)
		{
			strcat(new_content, line);
		}
		else
		{
			found++;
		}
	}
	fclose(fd);

	if (!found)
	{
		pthread_mutex_unlock(&borrowed_books_mutex);
		return 0;
	}

	FILE *fd2 = fopen("borrowed_books.txt", "w");
	fwrite(new_content, sizeof(char), strlen(new_content), fd2);
	fclose(fd2);

	pthread_mutex_unlock(&borrowed_books_mutex);

	pthread_mutex_lock(&books_mutex);

	char book_id[20];
	char *data_to_write = (char *)malloc(400);
	strcpy(data_to_write, buffer + 2);
	strcat(data_to_write, sep);

	char *token = strtok(buffer + 2, ",");
	strcpy(book_id, token);

	FILE *input_file = fopen("books.txt", "r");
	FILE *output_file = fopen("temp.txt", "w");

	char line2[400];

	while (fgets(line2, sizeof(line2), input_file) != NULL)
	{
		char *data = (char *)malloc(400);
		strcpy(data, line2);
		char *token2 = strtok(line2, ",");
		char current_book_id[20];
		strcpy(current_book_id, token2);
		if (strcmp(current_book_id, book_id) == 0)
		{
			char *name = strtok(NULL, ",");
			char *author = strtok(NULL, ",");
			char *count = strtok(NULL, ",");
			int num = atoi(count);
			if (num > 0)
			{
				num += 1;
				char new_count[20];
				sprintf(new_count, "%d", num);
				snprintf(line2, sizeof(line2), "%s,%s,%s,%s\n", book_id, name, author, new_count);
				fprintf(output_file, "%s", line2);
			}
		}

		else
		{
			fprintf(output_file, "%s", data);
		}
	}

	fclose(input_file);
	fclose(output_file);

	if (rename("temp.txt", "books.txt") != 0)
	{
		perror("Error renaming file");
		pthread_mutex_unlock(&books_mutex);
		return 0;
	}
	pthread_mutex_unlock(&books_mutex);
	return 1;
}

int add_member(char buffer[])
{
	pthread_mutex_lock(&logins_mutex);
	char sep[2] = "\n";
	FILE *fd = fopen("logins.txt", "a");
	fseek(fd, 0, SEEK_END);
	char data[400];
	strcpy(data, buffer + 2);
	strcat(data, sep);
	int data_written = fwrite(data, sizeof(char), strlen(data), fd);

	fclose(fd);
	pthread_mutex_unlock(&logins_mutex);
	if (data_written != 0)
		return 1;
	else
		return 0;
}

int login_member(char buffer[])
{
	pthread_mutex_lock(&logins_mutex);
	char sep[2] = "\n";
	FILE *fd = fopen("logins.txt", "r");
	char data[400];
	strcpy(data, buffer + 2);
	strcat(data, sep);
	int found = 0;
	char line[400];

	while (fgets(line, sizeof(line), fd) != NULL)
	{
		if (strcmp(line, data) == 0)
		{
			found = 1;
			break;
		}
	}
	pthread_mutex_unlock(&logins_mutex);
	return found;
}

void *handle_client(void *socket_fd)
{

	int new_socket = *((int *)socket_fd);
	ssize_t valread;
	char buffer[1024] = {0};
	while (1)
	{

		valread = read(new_socket, buffer,
					   1024 - 1);
		if (buffer[0] == '1')
		{ // Admin credentials to be checked
			char *message = "fail";
			int ret = check_admin_credentials(buffer);
			if (ret == 1)
				message = "success";
			memset(buffer, 0, sizeof(buffer));
			send(new_socket, message, strlen(message), 0);
		}

		if (buffer[0] == '2')
		{ // New book to be added
			char *message = "fail";
			int ret = add_book(buffer);
			if (ret == 1)
				message = "success";
			memset(buffer, 0, sizeof(buffer));
			send(new_socket, message, strlen(message), 0);
		}

		if (buffer[0] == '3')
		{ // Book to be removed
			char *message = "fail";
			int ret = remove_book_by_id(buffer);
			if (ret == 1)
				message = "success";
			memset(buffer, 0, sizeof(buffer));
			send(new_socket, message, strlen(message), 0);
		}

		if (buffer[0] == '4')
		{ // Book to be searched
			find_book(buffer, new_socket);
			memset(buffer, 0, sizeof(buffer));
		}

		if (buffer[0] == '5')
		{ // Book count modification request
			char *message = "fail";
			int ret = update_book_count(buffer);
			if (ret == 1)
			{
				message = "success";
			}
			memset(buffer, 0, sizeof(buffer));
			send(new_socket, message, strlen(message), 0);
		}

		if (buffer[0] == '6')
		{ // New member to be added
			char *message = "fail";
			int ret = add_member(buffer);
			if (ret == 1)
				message = "success";
			memset(buffer, 0, sizeof(buffer));
			send(new_socket, message, strlen(message), 0);
		}

		if (buffer[0] == '7')
		{ // Display all books
			display_books(buffer, new_socket);
			memset(buffer, 0, sizeof(buffer));
		}

		if (buffer[0] == '8')
		{ // Book to be borrowed
			char *message = "fail";
			int ret = borrow_book(buffer);
			if (ret == 1)
				message = "success";
			memset(buffer, 0, sizeof(buffer));
			send(new_socket, message, strlen(message), 0);
		}

		if (buffer[0] == '9')
		{ // Member login
			char *message = "fail";
			int ret = login_member(buffer);
			if (ret == 1)
				message = "success";
			memset(buffer, 0, sizeof(buffer));
			send(new_socket, message, strlen(message), 0);
		}

		if (buffer[0] == 'a')
		{ // Book to be borrowed
			char *message = "fail";
			int ret = return_book(buffer);
			if (ret == 1)
				message = "success";
			memset(buffer, 0, sizeof(buffer));
			send(new_socket, message, strlen(message), 0);
		}

		if (buffer[0] == 'b')
		{ // Display all members
			display_members(buffer, new_socket);
			memset(buffer, 0, sizeof(buffer));
		}

		if (buffer[0] == 'c')
		{ // Member to be removed
			char *message = "fail";
			int ret = remove_member(buffer);
			if (ret == 1)
				message = "success";
			memset(buffer, 0, sizeof(buffer));
			send(new_socket, message, strlen(message), 0);
		}
	}
	// closing the connected socket
	close(new_socket);
	pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
	int server_fd, new_socket;
	ssize_t valread;
	struct sockaddr_in address;
	int opt = 1;
	socklen_t addrlen = sizeof(address);
	char buffer[1024] = {0};
	char *hello = "Hello from server";

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET,
				   SO_REUSEADDR | SO_REUSEPORT, &opt,
				   sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address,
			 sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}
		pthread_t thread_id;
		if (pthread_create(&thread_id, NULL, handle_client, (void *)&new_socket) != 0)
		{
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}

	// closing the listening socket
	close(server_fd);
	return 0;
}
