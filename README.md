Chat App
Joshua Alexander Alex Robic

--Compling the program--
To compile, just run the make file by using the included makefile. It should generate 3 executables:
server -> the server program, no arguments required (uses port 8080 everytime)
client -> the client program, allows you to interact with the server, example usage: ./client {server_ip}
chat -> the program used to chat between 2 users. BOTH USERS HAVE TO BE ON THE SAME NETWORK! example usage: ./chat {destination_ip} {destination_name} {self_name} [password]
If a password is provided, the message will be encrypted using that password

--Running the program--
To run server, just run the compiled file server by typing "./server" in the terminal
To run client, run the compiled client file with the IP as a parameter ex: "./client 127.0.0.1"

--Inside client--
You have to have server running on the IP for client to work.
You can sign up by pressing "0" and creating a username and password.
You can also sign in by inputting a username and password that has already been created.

--Messaging a user--
When logged in, you can view the list of currently logged in users by pressing "0"
When you want to chat with a user, press "1" and then enter their username and a password.
*Waring*
The server fails if you enter an incorrect username at this part.
If the user is not connected to you, they get a request instead. The request has the username and attached password. 
If the password is incorrect, the real message is undecriptable. 

To see if anyone wants to chat with you, press "2" to view the list of requests.
ex: (elmo, donuts) -> a user by the name of elmo wants to chat with you, if you accept, connect using the client and use the password donuts
Lastly, press "9" to logout.
