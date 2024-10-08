# Online Library Management System

The Online Library Management System (OLMS) is built using system calls to perform diverse tasks such as managing processes, handling files, implementing file locking, managing multithreading, and facilitating interprocess communication. Socket programming is employed to establish client-server communication, enabling concurrent access to the library database by multiple clients.

## Server:

Implemented in library_server.c, the server handles multiple clients at a time (using multi-threading). All the requests received from the clients have a specific Request Number, that enables the server to identify the type of operation to be performed.

Mutex based locking ensures no clashes, when several clients try to access the same file.

## Client:

Implemented in library_client.c, the code provides a menu-based interface for the members/admin. Depending on the operation to be performed, a request is generated (that starts with the Request Number), and sent over to the server through the socket.

## admin_login.txt

Contains login credentials of the admin.

## books.txt

Contains details of the books in the library: (BookID, Book Name, Author, Number of Copies).

## borrowed_books.txt

Stores the transaction details for each book that has been borrowed: (BookID, Username)

## logins.txt

Stores login details of each member of the library.
