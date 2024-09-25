# OLMS
Simulating an Online Learning Management System in C language using system calls, Inter Process Communication using sockets, Concurrency Control using locks, OTP functionality through C language.


## Key Features

- Implements inter-process communication for seamless interaction between terminals.
- Provides concurrency control through file locks, allowing multiple users with varying access levels to read and write simultaneously.
- Includes OTP functionality to send one-time passwords to specified email addresses.
- Utilizes files as a database to store customer, admin, and library information.
- Organizes features across different files to enhance readability and maintainability of the OLMS.



## Getting Started

### Prerequisites
Make sure you have GCC installed on your system to compile the code.

### Running the Project

1. Clone the repository:
   ```bash
   git clone https://github.com/vrajnandak/Online-LMS.git
   ```
2. Change to the 'src' directory.
     ```bash
   cd github_proj_OS_LAB/src
     ```

3. Compile the server and client executables.
   ```bash
   gcc -o server server.c Admin_functions.c Customer_functions.c common_utils.c locking.c validate_functions.c Buffer_utils.c homepage.c logging.c structures.c
   gcc -o client client.c Buffer_utils.c common_utils.c
   ```

4. In separate terminal windows, run the following commands:
   ```bash
   ./server
   ./client
   ```
   
A connection to the server will be established automatically from the client, allowing you to access the Online Learning Management System.




## License

[MIT](https://choosealicense.com/licenses/mit/)

