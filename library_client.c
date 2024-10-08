#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#define PORT 8080

void display_main_menu()
{
	printf("*************Welcome to Library!***************\n");
	printf(" Enter 1 if you are the Library Administrator.\n");
	printf(" Enter 2 if you are an existing member.\n");
	printf(" Enter 3 if you are a new member.\n\n");
	printf(" Enter 0 to exit.\n\n");
}

void display_admin_menu()
{
	printf("*************Welcome to Library!***************\n");
	printf("1. Add new book.\n");
	printf("2. Delete existing book.\n");
	printf("3. Modify book count.\n");
	printf("4. Search for book details.\n");
	printf("5. Display all member details.\n");
	printf("6. Remove existing member.\n");
	printf("0. Enter 0 to logout.\n\n");
}

void display_member_menu()
{
	printf("*************Welcome to Library!***************\n");
	printf("1. Borrow book.\n");
	printf("2. Return book.\n\n");
	printf("0. Enter 0 to logout.\n\n");
}

void clear_input_buffer()
{
	int c;
	while ((c = getchar()) != '\n' && c != EOF)
		;
}

void add_book(int client_fd)
{
	char id[10];
	char name[50];
	char author[50];
	char count[5];
	char sep[2] = ",";
	char new_book[400] = "2"; // Request Number of adding book to library
	strcat(new_book, sep);

	printf("Enter Book ID: \n");
	scanf("%s", id);
	strcat(new_book, id);
	strcat(new_book, sep);

	clear_input_buffer(); // Clear input buffer after reading id

	printf("Enter Book Name: \n");
	fgets(name, sizeof(name), stdin);
	strtok(name, "\n");
	strcat(new_book, name);
	strcat(new_book, sep);

	printf("Enter Author Name: \n");
	fgets(author, sizeof(author), stdin);
	strtok(author, "\n");
	strcat(new_book, author);
	strcat(new_book, sep);

	printf("Enter count of Book: \n");
	scanf("%s", count);
	strcat(new_book, count);
	send(client_fd, new_book, strlen(new_book), 0);

	char buffer[20];
	read(client_fd, buffer, 10);

	if (!strcmp(buffer, "fail"))
	{
		printf("Book could not be added.\n\n");
		return;
	}
	printf("Book successfuly added!\n\n");
}

void delete_book(int client_fd)
{
	char delete_book[20] = "3,"; // Request Number
	char bid[20];
	printf("Enter ID of book to be deleted: \n");
	scanf("%s", bid);
	strcat(delete_book, bid);

	send(client_fd, delete_book, strlen(delete_book), 0);

	char buffer[20];
	read(client_fd, buffer, 20);

	if (!strcmp(buffer, "success"))
	{
		printf("Book successfully removed!\n\n");
		return;
	}
	printf("Book could not be removed.\n\n");
}

void update_book_count(int client_fd)
{
	char book_id[10];
	char new_count[5];
	char request[20] = "5,"; // Request Number
	char sep[2] = ",";

	printf("Enter Book ID: \n");
	scanf("%s", book_id);
	strcat(request, book_id);
	strcat(request, sep);

	printf("Enter new count of Book: \n");
	scanf("%s", new_count);
	strcat(request, new_count);

	send(client_fd, request, strlen(request), 0);

	char buffer[20];
	read(client_fd, buffer, 20);

	if (!strcmp(buffer, "success"))
	{
		printf("Book count successfully modified!\n\n");
	}
	else
	{
		printf("Failed to modify book count.\n\n");
	}
}

void search_book(int client_fd)
{
	char search_book[20] = "4,"; // Request Number
	char book_id[20];

	printf("Enter ID or name of the book to be searched: ");
	scanf("%s", book_id);
	strcat(search_book, book_id);

	send(client_fd, search_book, strlen(search_book), 0);

	char buffer[1024];
	ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);

	buffer[bytes_received] = '\0';
	printf("BookID,Book Name,Author Name,Copies: %s\n", buffer);
}

void show_members(int client_fd)
{
	char show_members[10] = "b"; // Request Number
	send(client_fd, show_members, strlen(show_members), 0);
	char buffer[1024];
	if (strlen(buffer) == 0)
		return;
	ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);

	buffer[bytes_received] = '\0';
	printf("Library Members:\nUsernames, Passwords:\n%s\n\n", buffer);
}

void show_books(int client_fd)
{
	char show_books[10] = "7"; // Request Number
	send(client_fd, show_books, strlen(show_books), 0);
	char buffer[1024];
	ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
	buffer[bytes_received] = '\0';
	printf("Books available:\nBookID,Book Name,Author Name,Copies:\n%s\n\n", buffer);
}

void delete_member(int client_fd)
{
	char delete_member[20] = "c,"; // Request Number
	char uname[20];
	printf("Enter username of member to be removed: \n");
	scanf("%s", uname);
	strcat(delete_member, uname);

	send(client_fd, delete_member, strlen(delete_member), 0);

	char buffer[20];
	read(client_fd, buffer, 20);

	if (!strcmp(buffer, "success"))
	{
		printf("Member successfully removed!\n\n");
		return;
	}
	printf("Member could not be removed.\n\n");
}

void borrow_book(int client_fd, char name[])
{
	show_books(client_fd);

	char book_id[10];
	char request[20] = "8,"; // Request Number
	char sep[2] = ",";

	printf("Enter Book ID: \n");
	scanf("%s", book_id);
	strcat(request, book_id);
	strcat(request, sep);
	strcat(request, name);
	send(client_fd, request, strlen(request), 0);

	char buffer[20];
	read(client_fd, buffer, 20);

	if (!strcmp(buffer, "success"))
	{
		printf("Book borrowed!\n\n");
	}
	else
	{
		printf("Failed to borrow book.\n\n");
	}
}

void return_book(int client_fd, char name[])
{
	show_books(client_fd);

	char book_id[10];
	char request[20] = "a,"; // Request Number
	char sep[2] = ",";

	printf("Enter Book ID: \n");
	scanf("%s", book_id);
	strcat(request, book_id);
	strcat(request, sep);
	strcat(request, name);
	send(client_fd, request, strlen(request), 0);

	char buffer[20];
	read(client_fd, buffer, 20);

	if (!strcmp(buffer, "success"))
	{
		printf("Book returned!\n\n");
	}
	else
	{
		printf("The book was never borrowed.\n\n");
	}
}

void admin_login(int client_fd)
{
	char name[20];
	char pwd[20];
	char sep[2] = ",";
	char request[200] = "1"; // Request Number
	strcat(request, sep);

	printf("Enter name: \n");
	scanf("%s", name);
	strcat(request, name);
	strcat(request, sep);

	printf("Enter password: \n");
	scanf("%s", pwd);
	strcat(request, pwd);

	send(client_fd, request, strlen(request), 0);
	char buffer[20];
	recv(client_fd, buffer, 10, 0);

	if (strcmp(buffer, "success") == 0)
	{
		printf("Login Successful!\n\n");

		while (1)
		{
			display_admin_menu();
			int choice;
			scanf("%d", &choice);
			if (choice == 1)
				add_book(client_fd);
			else if (choice == 2)
				delete_book(client_fd);
			else if (choice == 3)
				update_book_count(client_fd);
			else if (choice == 4)
				search_book(client_fd);
			else if (choice == 5)
				show_members(client_fd);
			else if (choice == 6)
				delete_member(client_fd);
			else if (choice == 0)
			{
				printf("Logging out...\n\n");
				return;
			}
			else
				printf("Invalid choice.\n\n");
		}
	}
	else
	{
		printf("Invalid credentials.\n\n");
		return;
	}
}

void existing_member_login(int client_fd)
{
	char name[20];
	char pwd[20];
	char sep[2] = ",";
	char request[200] = "9"; // Request Number
	strcat(request, sep);

	printf("Enter name: \n");
	scanf("%s", name);
	strcat(request, name);
	strcat(request, sep);

	printf("Enter password: \n");
	scanf("%s", pwd);
	strcat(request, pwd);

	send(client_fd, request, strlen(request), 0);

	char buffer[20];
	recv(client_fd, buffer, 10, 0);

	if (strcmp(buffer, "success") == 0)
	{
		printf("Login Successful!\n\n");

		while (1)
		{
			display_member_menu();
			int choice;
			scanf("%d", &choice);
			if (choice == 1)
				borrow_book(client_fd, name);
			else if (choice == 2)
				return_book(client_fd, name);
			else if (choice == 0)
			{
				printf("Logging out...\n\n");
				return;
			}
			else
				printf("Invalid choice.\n\n");
		}
	}
	else
	{
		printf("Invalid credentials.\n\n");
		return;
	}
}

void register_new_member(int client_fd)
{
	char name[20];
	char pwd[20];
	char sep[2] = ",";
	char request[200] = "6"; // Request Number
	strcat(request, sep);

	printf("Enter name: \n");
	scanf("%s", name);
	strcat(request, name);
	strcat(request, sep);

	printf("Enter password: \n");
	scanf("%s", pwd);
	strcat(request, pwd);

	send(client_fd, request, strlen(request), 0);
	char buffer[20];
	read(client_fd, buffer, 10);

	if (strcmp(buffer, "success") == 0)
	{
		printf("Member successfuly registered!\n\n");
		while (1)
		{
			display_member_menu();
			int choice;
			scanf("%d", &choice);
			if (choice == 1)
				borrow_book(client_fd, name);
			else if (choice == 2)
				return_book(client_fd, name);
			else if (choice == 0)
			{
				printf("Logging out...\n\n");
				return;
			}
			else
				printf("Invalid choice.\n\n");
		}
	}
	printf("Member could not be added.\n\n");
}

int main(int argc, char const *argv[])
{
	int status, valread, client_fd;
	struct sockaddr_in serv_addr;
	char *hello = "Hello from client";
	char buffer[1024] = {0};
	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
	{
		printf(
			"\nInvalid address/ Address not supported \n");
		return -1;
	}

	if ((status = connect(client_fd, (struct sockaddr *)&serv_addr,
						  sizeof(serv_addr))) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}

	while (1)
	{
		display_main_menu();
		int choice;
		scanf("%d", &choice);

		if (choice == 1)
			admin_login(client_fd);
		else if (choice == 2)
			existing_member_login(client_fd);
		else if (choice == 3)
			register_new_member(client_fd);
		else if (choice == 0)
			exit(0);

		else
			printf("Invalid choice.\n\n");
	}

	// closing the connected socket
	close(client_fd);
	return 0;
}
