#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include"struct.h"

#define PORT 8082
#define LEN 50
//-------------------------------------------------------------------FUNCTION DECLARATIONS----------------------------------------------------
void client_service(int scktDesc);
void signIN(int scktDesc);
void signUP(int scktDesc);

int MainMenu(int scktDesc,int acntType,int login_id);

void addTrain(int);
void viewTrain(int);
void updateTrain(int);
void deleteTrain(int);
void Opertn_train(int scktDesc);

void addUser(int);
void viewUser(int);
void updateUser(int);
void deleteUser(int);
void Opertn_user(int scktDesc);

int bookTicket(int,int,int);
int viewBooking(int,int,int);
int updateBooking(int,int,int);
int cancelBooking(int,int,int);
int user_function(int scktDesc,int choice,int acntType,int login_id);
   
//------------------------------------------------------------------------ENTRY POINT-----------------------------------------------------------
void client_service(int scktDesc){
	int choice;
	printf("\nClient [%d] Connected\n", scktDesc);
	do{
		read(scktDesc, &choice, sizeof(int));		
		switch(choice){
			case 1:signIN(scktDesc);
				break;
			case 2:signUP(scktDesc);
				break;
			case 3: break;
			default:client_service(scktDesc);
		}
	}while(1);

	close(scktDesc);
	printf("\n\tClient [%d] Disconnected\n", scktDesc);
}

//----------------------------------------------------------------------------SIGN IN  -------------------------------------------------------//

void signIN(int scktDesc){
	int fd_user = open("user_db",O_RDWR);
	int login_id,acntType,valid=0,user_valid=0;
	char password[LEN];
	struct user u1;
	
	read(scktDesc,&login_id,sizeof(login_id));
	read(scktDesc,&password,sizeof(password));
	
	struct flock lock;
	
	lock.l_start = (login_id-1)*sizeof(struct user);
	lock.l_len = sizeof(struct user);
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	lock.l_type = F_WRLCK;
	fcntl(fd_user,F_SETLKW, &lock);
	
	while(read(fd_user,&u1,sizeof(u1))){
		if(u1.login_id==login_id){
			user_valid=1;
			if(!strcmp(u1.password,password)){
				valid = 1;
				acntType = u1.type;
				break;
			}
			else{
				valid = 0;
				break;
			}	
		}		
		else{
			user_valid = 0;
			valid=0;
		}
	}
	
	
	if(acntType!=2){
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		close(fd_user);
	}
	
	// if valid user, show him menu
	if(user_valid)
	{
		write(scktDesc,&valid,sizeof(valid));
		if(valid){
			write(scktDesc,&acntType,sizeof(acntType));
			while(MainMenu(scktDesc,acntType,login_id)!=-1);
		}
	}
	else
		write(scktDesc,&valid,sizeof(valid));
	
	
	if(acntType==2){
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		close(fd_user);
	}
} 

//----------------------------------------------------------------------- SIGN UP -------------------------------------------------------------------//

void signUP(int scktDesc){
	int fd_user = open("user_db",O_RDWR);
	int acntType,login_id=0;
	char name[LEN],password[LEN];
	struct user u,temp;

	read(scktDesc, &acntType, sizeof(acntType));
	read(scktDesc, &name, sizeof(name));
	read(scktDesc, &password, sizeof(password));

	int fp = lseek(fd_user, 0, SEEK_END);

	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = fp;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();


	fcntl(fd_user,F_SETLKW, &lock);

	
	if(fp==0){
		u.login_id = 1;
		u.type=acntType;
		strcpy(u.name, name);
		strcpy(u.password, password);
		write(fd_user, &u, sizeof(u));
		write(scktDesc, &u.login_id, sizeof(u.login_id));
	}
	else{
		fp = lseek(fd_user, -1 * sizeof(struct user), SEEK_END);
		read(fd_user, &u, sizeof(u));
		u.login_id++;
		strcpy(u.name, name);
		strcpy(u.password, password);
		u.type=acntType;
		write(fd_user, &u, sizeof(u));
		write(scktDesc, &u.login_id, sizeof(u.login_id));
	}
	lock.l_type = F_UNLCK;
	fcntl(fd_user, F_SETLK, &lock);

	close(fd_user);
	
}

//----------------------------------------------------------------------- Main menu function---------------------------------------------------------//

int MainMenu(int scktDesc,int acntType,int login_id){
	int choice,ret;

	// for admin
	if(acntType==0){
		read(scktDesc,&choice,sizeof(choice));
		switch(choice)
		{
			case 1: Opertn_train(scktDesc);
				return MainMenu(scktDesc,acntType,login_id);
				
			case 2: Opertn_user(scktDesc);
				return MainMenu(scktDesc,acntType,login_id);
				
			case 3:return -1;
		}
	}
	else if(acntType==1 || acntType==2){				// For agent and customer
		read(scktDesc,&choice,sizeof(choice));
		ret = user_function(scktDesc,choice,acntType,login_id);
		if(ret!=5)
			return MainMenu(scktDesc,acntType,login_id);
		else if(ret==5)
			return -1;
	}		
}

//------------------------------------------------------------------------- Operation on train--------------------------------------------------------//

//---------------------------------------------------------------------------ADD TRAIN-------------------------------------------------------------

void addTrain(int client_sock)
{
	char tname[LEN],valid=0;
	int tid = 0;
	read(client_sock,&tname,sizeof(tname));
	struct train tdb,temp;
	struct flock lock;
	int fd_train = open("train_db", O_RDWR);
	
	tdb.train_number = tid;
	strcpy(tdb.train_name,tname);
	tdb.total_seats = 10;				// by default, we are taking 10 seats
	tdb.available_seats = 10;

	int fp = lseek(fd_train, 0, SEEK_END); 
	lock.l_type = F_WRLCK;
	lock.l_start = fp;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();
	fcntl(fd_train, F_SETLKW, &lock);

	if(fp == 0){
		valid = 1;
		write(fd_train, &tdb, sizeof(tdb));
		lock.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lock);
		close(fd_train);
		write(client_sock, &valid, sizeof(valid));
	}
	else{
		valid = 1;
		lseek(fd_train, -1 * sizeof(struct train), SEEK_END);
		read(fd_train, &temp, sizeof(temp));
		tdb.train_number = temp.train_number + 1;
		write(fd_train, &tdb, sizeof(tdb));
		write(client_sock, &valid,sizeof(valid));	
	}
	lock.l_type = F_UNLCK;
	fcntl(fd_train, F_SETLK, &lock);
	close(fd_train);
}

//----------------------------------------------------------------------------------------VIEW TRAIN------------------------------------------------------
void viewTrain(int client_sock)
{
	struct flock lock;
	struct train tdb;
	int valid=0;
	int fd_train = open("train_db", O_RDONLY);
	
	lock.l_type = F_RDLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();
		
	fcntl(fd_train, F_SETLKW, &lock);
	int fp = lseek(fd_train, 0, SEEK_END);
	int no_of_trains = fp / sizeof(struct train);
	write(client_sock, &no_of_trains, sizeof(int));
	lseek(fd_train,0,SEEK_SET);
	while(fp != lseek(fd_train,0,SEEK_CUR)){
		read(fd_train,&tdb,sizeof(tdb));
		write(client_sock,&tdb.train_number,sizeof(int));
		write(client_sock,&tdb.train_name,sizeof(tdb.train_name));
		write(client_sock,&tdb.total_seats,sizeof(int));
		write(client_sock,&tdb.available_seats,sizeof(int));
	}
	valid = 1;
	lock.l_type = F_UNLCK;
	fcntl(fd_train, F_SETLK, &lock);
	close(fd_train);
}

//-------------------------------------------------------------------------------------UPDATE TRAIN---------------------------------------------------
void updateTrain(int client_sock)
{
	Opertn_train(client_sock);
	int choice,valid=0,tid;
	struct flock lock;
	struct train tdb;
	int fd_train = open("train_db", O_RDWR);

	read(client_sock,&tid,sizeof(tid));
	lock.l_type = F_WRLCK;
	lock.l_start = (tid)*sizeof(struct train);
	lock.l_len = sizeof(struct train);
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();
		
	fcntl(fd_train, F_SETLKW, &lock);
	lseek(fd_train, 0, SEEK_SET);
	lseek(fd_train, (tid)*sizeof(struct train), SEEK_CUR);
	read(fd_train, &tdb, sizeof(struct train));
		
	read(client_sock,&choice,sizeof(int));
	if(choice==1){							// update train name
		write(client_sock,&tdb.train_name,sizeof(tdb.train_name));
		read(client_sock,&tdb.train_name,sizeof(tdb.train_name));
		
	}
	else if(choice==2){						// update total number of seats
		write(client_sock,&tdb.total_seats,sizeof(tdb.total_seats));
		read(client_sock,&tdb.total_seats,sizeof(tdb.total_seats));
	}
	
	lseek(fd_train, -1*sizeof(struct train), SEEK_CUR);
	write(fd_train, &tdb, sizeof(struct train));
	valid=1;
	write(client_sock,&valid,sizeof(valid));
	lock.l_type = F_UNLCK;
	fcntl(fd_train, F_SETLK, &lock);
	close(fd_train);
}

int checkAccountExists(int id)
{
	struct train db;

	int fd1 = open("train_db", O_RDWR );
	
	while(read(fd1, (char *)&db, sizeof(struct train)))
	{

		if(db.train_number==id)
			{
				if(strcmp("deleted",db.train_name)){
				close(fd1);
				return 1;}
				else	return 0;
			}
	}
	close(fd1);
	return 0;	
	
}

//------------------------------------------------------------------------------------DELETE TRAIN-------------------------------------------------------


void deleteTrain(int client_sock)
{
	Opertn_train(client_sock);
	struct flock lock;
	struct train tdb;
	int fd_train = open("train_db", O_RDWR);
	int tid,valid=0;

	read(client_sock,&tid,sizeof(tid));
	
	
	if(!checkAccountExists(tid))
	{	valid=0;
		write(client_sock,&valid, sizeof(int));
	}
	else
	{	

		lock.l_type = F_WRLCK;
		lock.l_start = (tid)*sizeof(struct train);
		lock.l_len = sizeof(struct train);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_train, F_SETLKW, &lock);
		
		lseek(fd_train, 0, SEEK_SET);
		lseek(fd_train, (tid)*sizeof(struct train), SEEK_CUR);
		read(fd_train, &tdb, sizeof(struct train));
		strcpy(tdb.train_name,"deleted");
		lseek(fd_train, -1*sizeof(struct train), SEEK_CUR);
		write(fd_train, &tdb, sizeof(struct train));
		valid=1;
		write(client_sock,&valid,sizeof(valid));
		lock.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lock);
		
	}

	close(fd_train);

	
}

void Opertn_train(int scktDesc){
	int valid=0;	
	int choice;
	read(scktDesc,&choice,sizeof(choice));
	
	switch(choice)
	{
	//------------ADD TRAIN------------------
	case 1:  addTrain(scktDesc);
		 break;

	//------------VIEW TRAIN-----------------
	case 2:  viewTrain(scktDesc);
		 break;
	
	//------------UPDATE TRAIN---------------
	case 3:  updateTrain(scktDesc);
		 break;
		 
	//------------DELETE TRAIN---------------
	case 4: deleteTrain(scktDesc);
		break;
	}


}

//---------------------------------------------------------------------------- Operation on user--------------------------------------------------------//

//---------------------------------------------------------------------------ADD USER----------------------------------------------------------------------
void addUser(int client_sock)
{
	char name[LEN],password[LEN];
	int type,valid=0;
	read(client_sock, &type, sizeof(type));
	read(client_sock, &name, sizeof(name));
	read(client_sock, &password, sizeof(password));
		
	struct user udb;
	struct flock lock;
	int fd_user = open("user_db", O_RDWR);
	int fp = lseek(fd_user, 0, SEEK_END);
		
	lock.l_type = F_WRLCK;
	lock.l_start = fp;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd_user, F_SETLKW, &lock);

	if(fp==0){
		udb.login_id = 1;
		strcpy(udb.name, name);
		strcpy(udb.password, password);
		udb.type=type;
		write(fd_user, &udb, sizeof(udb));
		valid = 1;
		write(client_sock,&valid,sizeof(int));
		write(client_sock, &udb.login_id, sizeof(udb.login_id));
	}
	else{
		fp = lseek(fd_user, -1 * sizeof(struct user), SEEK_END);
		read(fd_user, &udb, sizeof(udb));
		udb.login_id++;
		strcpy(udb.name, name);
		strcpy(udb.password, password);
		udb.type=type;
		write(fd_user, &udb, sizeof(udb));
		valid = 1;
		write(client_sock,&valid,sizeof(int));
		write(client_sock, &udb.login_id, sizeof(udb.login_id));
	}
	lock.l_type = F_UNLCK;
	fcntl(fd_user, F_SETLK, &lock);
	close(fd_user);
}

//------------------------------------------------------------------------------VIEW USER-----------------------------------------------------------------
void viewUser(int client_sock)
{
			
	struct flock lock;
	struct user udb;
	int valid=0;
	int fd_user = open("user_db", O_RDONLY);
	
	lock.l_type = F_RDLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();
		
	fcntl(fd_user, F_SETLKW, &lock);
	int fp = lseek(fd_user, 0, SEEK_END);
	int no_of_users= fp / sizeof(struct user);
	write(client_sock, &no_of_users, sizeof(int));
	lseek(fd_user,0,SEEK_SET);
	while(fp != lseek(fd_user,0,SEEK_CUR)){
		read(fd_user,&udb,sizeof(udb));
		write(client_sock,&udb.login_id,sizeof(int));
		write(client_sock,&udb.name,sizeof(udb.name));
		write(client_sock,&udb.type,sizeof(int));
	}
	valid = 1;
	lock.l_type = F_UNLCK;
	fcntl(fd_user, F_SETLK, &lock);
	close(fd_user);
}
int checkUserExists(int id)
{
	struct user db;

	int fd1 = open("user_db", O_RDWR );
	
	while(read(fd1, (char *)&db, sizeof(struct user)))
	{

		if(db.login_id==id)
			{
				if(strcmp("deleted",db.name)){
					close(fd1);
					return 1;
				}
				else{	
					close(fd1);	
					return 0;
				}
			}
	}
	close(fd1);
	return 0;	
	
}

//----------------------------------------------------------------------------UPDATE USER-----------------------------------------------------------------------
void updateUser(int client_sock)
{
	Opertn_user(client_sock);
	int choice,valid=0,uid;
	char pass[50];
	struct flock lock;
	struct user udb;
	int fd_user = open("user_db", O_RDWR);

	read(client_sock,&uid,sizeof(uid));


	lock.l_type = F_WRLCK;
	lock.l_start =  (uid-1)*sizeof(struct user);
	lock.l_len = sizeof(struct user);
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();
		
	fcntl(fd_user, F_SETLKW, &lock);

	lseek(fd_user, 0, SEEK_SET);
	lseek(fd_user, (uid-1)*sizeof(struct user), SEEK_CUR);
	read(fd_user, &udb, sizeof(struct user));
		
	read(client_sock,&choice,sizeof(int));
	if(choice==1){					// update name
		write(client_sock,&udb.name,sizeof(udb.name));
		read(client_sock,&udb.name,sizeof(udb.name));
		valid=1;
		write(client_sock,&valid,sizeof(valid));		
	}
	else if(choice==2){				// update password
		read(client_sock,&pass,sizeof(pass));
		if(!strcmp(udb.password,pass))
			valid = 1;
		write(client_sock,&valid,sizeof(valid));
		read(client_sock,&udb.password,sizeof(udb.password));
	}
	
	lseek(fd_user, -1*sizeof(struct user), SEEK_CUR);
	write(fd_user, &udb, sizeof(struct user));
	if(valid)
		write(client_sock,&valid,sizeof(valid));
	lock.l_type = F_UNLCK;
	fcntl(fd_user, F_SETLK, &lock);
	
	close(fd_user);
}	
//----------------------------------------------------------------------------------------DELETE USER------------------------------------------------




void deleteUser(int client_sock)
{
	Opertn_user(client_sock);
	struct flock lock;
	struct user udb;
	int fd_user = open("user_db", O_RDWR);
	int uid,valid=0;

	read(client_sock,&uid,sizeof(uid));
	
	if(!checkUserExists(uid)){	
		valid=0;
		write(client_sock,&valid, sizeof(int));
	}
	else
	{	
		

		lock.l_type = F_WRLCK;
		lock.l_start =  (uid-1)*sizeof(struct user);
		lock.l_len = sizeof(struct user);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
			
		fcntl(fd_user, F_SETLKW, &lock);
			
		lseek(fd_user, 0, SEEK_SET);
		lseek(fd_user, (uid-1)*sizeof(struct user), SEEK_CUR);
		read(fd_user, &udb, sizeof(struct user));
		strcpy(udb.name,"deleted");
		strcpy(udb.password,"");
		lseek(fd_user, -1*sizeof(struct user), SEEK_CUR);
		write(fd_user, &udb, sizeof(struct user));
		valid=1;
		write(client_sock,&valid,sizeof(valid));
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
	}
	close(fd_user);	
}


void Opertn_user(int scktDesc){
	int valid=0;	
	int choice;
	read(scktDesc,&choice,sizeof(choice));
	
	switch(choice)
	{
	//--------------ADD USER------------------
	case 1: addUser(scktDesc);
		break;
	//--------------VIEW USER----------------
	case 2: viewUser(scktDesc);
		break;
	//--------------UPDATE USER--------------
	case 3: updateUser(scktDesc);
		break;
	//--------------DELETE USER--------------
	case 4: deleteUser(scktDesc);
		break;
	}	
	

}


//-------------------------------------------------------------------------- User functions -----------------------------------------------//
//----------------------------------------------------------------------------BOOK TICKET---------------------------------------------------------------

int bookTicket(int scktDesc,int type,int id)
{
	int valid=0;
	Opertn_train(scktDesc);
	struct flock lockt;
	struct flock lockb;
	struct train tdb;
	struct booking bdb;
	int fd_train = open("train_db", O_RDWR);
	int fd_book = open("booking_db", O_RDWR);
	int tid,seats;
	read(scktDesc,&tid,sizeof(tid));		
			
	lockt.l_type = F_WRLCK;
	lockt.l_start = tid*sizeof(struct train);
	lockt.l_len = sizeof(struct train);
	lockt.l_whence = SEEK_SET;
	lockt.l_pid = getpid();
		
	lockb.l_type = F_WRLCK;
	lockb.l_start = 0;
	lockb.l_len = 0;
	lockb.l_whence = SEEK_END;
	lockb.l_pid = getpid();
		
	fcntl(fd_train, F_SETLKW, &lockt);
	lseek(fd_train,tid*sizeof(struct train),SEEK_SET);
		
	read(fd_train,&tdb,sizeof(tdb));
	read(scktDesc,&seats,sizeof(seats));

	if(tdb.train_number==tid)
	{		
		if(tdb.available_seats>=seats){
			valid = 1;
			tdb.available_seats -= seats;
			fcntl(fd_book, F_SETLKW, &lockb);
			int fp = lseek(fd_book, 0, SEEK_END);
			
			if(fp > 0){
				lseek(fd_book, -1*sizeof(struct booking), SEEK_CUR);
				read(fd_book, &bdb, sizeof(struct booking));
				bdb.booking_id++;
			}
			else 
				bdb.booking_id = 0;

			bdb.type = type;
			bdb.uid = id;
			bdb.tid = tid;
			bdb.seats = seats;
			write(fd_book, &bdb, sizeof(struct booking));
			lockb.l_type = F_UNLCK;
			fcntl(fd_book, F_SETLK, &lockb);
		 	close(fd_book);
		}
	
	lseek(fd_train, -1*sizeof(struct train), SEEK_CUR);
	write(fd_train, &tdb, sizeof(tdb));
	}

	lockt.l_type = F_UNLCK;
	fcntl(fd_train, F_SETLK, &lockt);
	close(fd_train);
	write(scktDesc,&valid,sizeof(valid));
	return valid;		
}
//-------------------------------------------------------------------------------------VIEW BOOKING----------------------------------------------------
int viewBooking(int scktDesc,int type,int id)
{
	int valid=0;
	struct flock lock;
	struct booking bdb;
	int fd_book = open("booking_db", O_RDONLY);
	int no_of_bookings = 0;
	
	lock.l_type = F_RDLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();
		
	fcntl(fd_book, F_SETLKW, &lock);
	
	while(read(fd_book,&bdb,sizeof(bdb))){
		if (bdb.uid==id)
			no_of_bookings++;
	}

	write(scktDesc, &no_of_bookings, sizeof(int));
	lseek(fd_book,0,SEEK_SET);

	while(read(fd_book,&bdb,sizeof(bdb))){
		if(bdb.uid==id){
			write(scktDesc,&bdb.booking_id,sizeof(int));
			write(scktDesc,&bdb.tid,sizeof(int));
			write(scktDesc,&bdb.seats,sizeof(int));
		}
	}
	lock.l_type = F_UNLCK;
	fcntl(fd_book, F_SETLK, &lock);
	close(fd_book);
	return valid;
}

//---------------------------------------------------------------------------------------UPDATE BOOKING-----------------------------------------------------

int updateBooking(int scktDesc,int type,int id)
{
	int choice = 2,bid,val;
	int valid=0;
	user_function(scktDesc,choice,type,id);
	struct booking bdb;
	struct train tdb;
	struct flock lockb;
	struct flock lockt;
	int fd_book = open("booking_db", O_RDWR);
	int fd_train = open("train_db", O_RDWR);
	read(scktDesc,&bid,sizeof(bid));

	lockb.l_type = F_WRLCK;
	lockb.l_start = bid*sizeof(struct booking);
	lockb.l_len = sizeof(struct booking);
	lockb.l_whence = SEEK_SET;
	lockb.l_pid = getpid();
		
	fcntl(fd_book, F_SETLKW, &lockb);
	lseek(fd_book,bid*sizeof(struct booking),SEEK_SET);
	read(fd_book,&bdb,sizeof(bdb));
	lseek(fd_book,-1*sizeof(struct booking),SEEK_CUR);
		
	lockt.l_type = F_WRLCK;
	lockt.l_start = (bdb.tid)*sizeof(struct train);
	lockt.l_len = sizeof(struct train);
	lockt.l_whence = SEEK_SET;
	lockt.l_pid = getpid();

	fcntl(fd_train, F_SETLKW, &lockt);
	lseek(fd_train,(bdb.tid)*sizeof(struct train),SEEK_SET);
	read(fd_train,&tdb,sizeof(tdb));
	lseek(fd_train,-1*sizeof(struct train),SEEK_CUR);

	read(scktDesc,&choice,sizeof(choice));
	
	if(choice==1){							// increase number of seats required of booking id
		read(scktDesc,&val,sizeof(val));
		if(tdb.available_seats>=val){
			valid=1;
			tdb.available_seats -= val;
			bdb.seats += val;
		}
	}
	else if(choice==2){						// decrease number of seats required of booking id
		valid=1;
		read(scktDesc,&val,sizeof(val));
		tdb.available_seats += val;
		bdb.seats -= val;	
	}
		
	write(fd_train,&tdb,sizeof(tdb));
	lockt.l_type = F_UNLCK;
	fcntl(fd_train, F_SETLK, &lockt);
	close(fd_train);
		
	write(fd_book,&bdb,sizeof(bdb));
	lockb.l_type = F_UNLCK;
	fcntl(fd_book, F_SETLK, &lockb);
	close(fd_book);
		
	write(scktDesc,&valid,sizeof(valid));
	return valid;
}
//------------------------------------------------------------------------------CANCEL BOOKING----------------------------------------------------

int cancelBooking(int scktDesc,int type,int id)
{
	int choice = 2,bid;
	int valid=0;
	user_function(scktDesc,choice,type,id);
	struct booking bdb;
	struct train tdb;
	struct flock lockb;
	struct flock lockt;
	int fd_book = open("booking_db", O_RDWR);
	int fd_train = open("train_db", O_RDWR);
	read(scktDesc,&bid,sizeof(bid));

	lockb.l_type = F_WRLCK;
	lockb.l_start = bid*sizeof(struct booking);
	lockb.l_len = sizeof(struct booking);
	lockb.l_whence = SEEK_SET;
	lockb.l_pid = getpid();
		
	fcntl(fd_book, F_SETLKW, &lockb);
	lseek(fd_book,bid*sizeof(struct booking),SEEK_SET);
	read(fd_book,&bdb,sizeof(bdb));
	lseek(fd_book,-1*sizeof(struct booking),SEEK_CUR);
		
	lockt.l_type = F_WRLCK;
	lockt.l_start = (bdb.tid)*sizeof(struct train);
	lockt.l_len = sizeof(struct train);
	lockt.l_whence = SEEK_SET;
	lockt.l_pid = getpid();

	fcntl(fd_train, F_SETLKW, &lockt);
	lseek(fd_train,(bdb.tid)*sizeof(struct train),SEEK_SET);
	read(fd_train,&tdb,sizeof(tdb));
	lseek(fd_train,-1*sizeof(struct train),SEEK_CUR);

	tdb.available_seats += bdb.seats;
	bdb.seats = 0;
	valid = 1;

	write(fd_train,&tdb,sizeof(tdb));
	lockt.l_type = F_UNLCK;
	fcntl(fd_train, F_SETLK, &lockt);
	close(fd_train);
		
	write(fd_book,&bdb,sizeof(bdb));
	lockb.l_type = F_UNLCK;
	fcntl(fd_book, F_SETLK, &lockb);
	close(fd_book);
		
	write(scktDesc,&valid,sizeof(valid));
	return valid;
}
int user_function(int scktDesc,int choice,int type,int id){
	int valid=0;
	
	switch(choice)
	{
	//-------------------BOOK TICKET--------------
	case 1: valid=bookTicket(scktDesc,type,id);
		return valid;	
	
	//------------------VIEW BOOKING--------------
	case 2: valid=viewBooking(scktDesc,type,id);
		return valid;
	
	//------------------UPDATE BOOKING-------------
	case 3: valid=updateBooking(scktDesc,type,id);
		return valid;
	
	//------------------CANCEL BOOKING------------
	case 4: valid=cancelBooking(scktDesc,type,id);
		return valid;
	
	//-----------------LOGOUT----------------------
	case 5: return 5;
	}
	
}
