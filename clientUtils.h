#include <string.h>
#include <sys/socket.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <stdio.h>
#include <stdlib.h> 

#include"struct.h"

#define PORT 8082
#define LEN 50


//-------------------------FUNCTION DECLARATIONS---------------------

int client_Entry(int scktDesc);
int signIN(int scktDesc);
int signUP(int scktDesc);

int MainMenu(int scktDesc,int acntType);
int user_function(int scktDesc,int choice);
int addTrain(int);
int viewTrain(int);
int updateTrain(int);
int deleteTrain(int);
int Opertn_train(int scktDesc,int choice);

int addUser(int);
int viewUser(int);
int updateUser(int);
int deleteUser(int);
int Opertn_user(int scktDesc,int choice);

//------------------------------------------------------------Entry point-------------------------------------------------------

int client_Entry(int scktDesc){
	int choice,valid;
	
	printf("-------------------------WELCOME--------------------------\n");
	printf("\n---------------------TRAIN RESERVATION SYSTEM--------------------------\n");
	printf("Choose from below:\n");
	printf("1. SIGN IN\n");
	printf("2. SIGN UP\n");
	printf("3. EXIT\n");
	printf("Enter Your Choice: ");
	scanf("%d", &choice);
	//write choice into socket
	write(scktDesc, &choice, sizeof(choice));
	
	int ret;
	switch(choice)
	{
		case 1: ret=signIN(scktDesc);   //------SIGN IN--------//
			return ret;
			
		case 2: ret=signUP(scktDesc);	//-------SIGN UP--------//
			return ret;
			
		case 3: return 3;		//-------LOGOUT---------//
		
		default: printf("Enter correct option\n");
			ret=client_Entry(scktDesc);
			return ret;
	} 
}

//---------------------------------------------------------------SIGN IN-----------------------------------------------------------------//
int signIN(int scktDesc){
	int login_id,acnt_type,valid;
	char password[LEN];
	printf("--------------------------SIGN IN---------------------\n");
	printf("Enter login id: ");
	scanf("%d", &login_id);
	strcpy(password,getpass("Enter password: "));
	//write login id and password onto socket
	write(scktDesc, &login_id, sizeof(login_id));
	write(scktDesc, &password, sizeof(password));
	//read whether given credentials are valid
	read(scktDesc, &valid, sizeof(valid));
	
	if(valid){
		printf("\n-----------------Login successfull------------\n");
		read(scktDesc,&acnt_type,sizeof(acnt_type));
		while(MainMenu(scktDesc,acnt_type)!=-1);
		return 1;
	}
	else{
		printf("\n------------Login Failed-------------\n");
		printf("Enter correct user id and password\n");
		return 1;
	}
}

//--------------------------------------------------------------SIGN UP------------------------------------------------------------------//
int signUP(int scktDesc)
{
	int acnt_type,login_id,valid;
	char name[LEN],password[LEN];
	system("clear");
	printf("-------------------------SIGN UP---------------------\n");
	printf("Choose Account type from below\n");
	
	printf("0. Administrator\n1. Agent Account\n2. Customer Account\n");
	//Enter credentials
	printf("Enter  Type Of your Account: \n");
	scanf("%d", &acnt_type);
	printf("Enter Your Name: ");
	scanf("%s", name);
	strcpy(password,getpass("Enter Password: "));

	//write credentials onto socket 	
	write(scktDesc, &acnt_type, sizeof(acnt_type));
	write(scktDesc, &name, sizeof(name));
	write(scktDesc, &password, strlen(password));
	//read login id from socket
	read(scktDesc, &login_id, sizeof(login_id));
	
	printf("\nSign Up successful\n Your login ID is : %d\n", login_id);
	return 2;
}




//------------------------------------------------------------------- Main menu function-----------------------------------------------------------//

int MenuAdmin()
{
	int choice;
	printf("Choose from below\n");
	printf("\n1.Operations on train\n");
	printf("2. Operations on user\n");
	printf("3. Logout\n");
	printf("Enter Your Choice: ");
	scanf("%d",&choice);
	return choice;
}
int MenuCust()
{
	int choice;
	printf("\n\n------OPTIONS--------\n");
	printf("1. Book Ticket\n");
	printf("2. View Bookings\n");
	printf("3. Update Booking\n");
	printf("4. Cancel booking\n");
	printf("5. Logout\n");
	printf("\nEnter Your Choice: ");
	scanf("%d",&choice);
	return choice;

}

int details(int i){
	char *str;
	int choice;
	if(i==1)	str="train";
	else str="user";
	printf("1. Add %s\n",str);
	printf("2. View %s\n",str);
	printf("3. Modify %s\n",str);
	printf("4. Delete %s\n",str);
	printf("Enter Your Choice: ");
	scanf("%d",&choice);	
	return choice;
}

int MainMenu(int scktDesc,int type){
	int choice;
	if(type==1 || type==2){
		choice=MenuCust();
		write(scktDesc,&choice,sizeof(choice));
		return user_function(scktDesc,choice);
	}
	else if(type==0){					// Admin
		choice=MenuAdmin();
		write(scktDesc,&choice,sizeof(choice));
		if(choice==1)
		{
			choice=details(choice);
			write(scktDesc,&choice,sizeof(choice));
			return Opertn_train(scktDesc,choice);
		}
		else if(choice==2)
		{
			choice=details(choice);
			write(scktDesc,&choice,sizeof(choice));
			return Opertn_user(scktDesc,choice);
		}
		else if(choice==3)
			return -1;
	}	
	
}

//--------------------------------------------------------------------- Operations on train---------------------------------------------------------------//


//-------------------------------------------------------------------------ADD TRAIN---------------------------------------------------
int addTrain(int scktDesc)
{
	char tname[LEN];
	int valid;
	printf("\nEnter train name: ");
	scanf("%s",tname);
	write(scktDesc, &tname, sizeof(tname));
	read(scktDesc,&valid,sizeof(valid));	
	if(valid)
		printf("\nTrain added successfully\n");
	return valid;	
}
//-----------------------------------------------------------------------VIEW TRAIN-------------------------------------------------------
int viewTrain(int scktDesc)
{
	int no_of_trains;
	int tno;
	char tname[LEN];
	int tot_seats;
	int avail_seats;
	int valid=0;
	
	read(scktDesc,&no_of_trains,sizeof(no_of_trains));

	printf("\tTrain no\tTrain name\tTotal seats\tAvail seats\n");
	while(no_of_trains--){
		read(scktDesc,&tno,sizeof(tno));
		read(scktDesc,&tname,sizeof(tname));
		read(scktDesc,&tot_seats,sizeof(tot_seats));
		read(scktDesc,&avail_seats,sizeof(avail_seats));
			
		if(strcmp(tname, "deleted")!=0)
			printf("\t%d\t\t%s\t\t%d\t\t%d\n",tno,tname,tot_seats,avail_seats);
	}
	return valid;
}
//--------------------------------------------------------------------UPDATE TRAIN---------------------------------------------------
int updateTrain(int scktDesc)
{
	int tot_seats,choice=2,valid=0,tid;
	char tname[LEN];
	write(scktDesc,&choice,sizeof(int));
	
	Opertn_train(scktDesc,choice);
	
	printf("\n Enter the train number you want to modify: ");
	scanf("%d",&tid);
	
	write(scktDesc,&tid,sizeof(tid));
	
	printf("Choose below to update\n");
	printf("\n1. Train Name\n\t2. Total Seats\n");
	printf(" Your Choice: ");
	scanf("%d",&choice);
	
	write(scktDesc,&choice,sizeof(choice));
	
	if(choice==1){
		read(scktDesc,&tname,sizeof(tname));
		printf("\n Current name: %s",tname);
		printf("\n Updated name:");
		scanf("%s",tname);
		write(scktDesc,&tname,sizeof(tname));
	}
	else if(choice==2){
		read(scktDesc,&tot_seats,sizeof(tot_seats));
		printf("\n Current value: %d",tot_seats);
		printf("\n Updated value:");
		scanf("%d",&tot_seats);
		write(scktDesc,&tot_seats,sizeof(tot_seats));
	}
	
	read(scktDesc,&valid,sizeof(valid));
	
	if(valid)
		printf("\n Train data updated successfully\n");
	else
		printf("Error in updating\n");

	return valid;
}

//--------------------------------------------------------------------------DELETE TRAIN------------------------------------------------
int deleteTrain(int scktDesc)
{
	int choice=2,tid,valid=0;
	write(scktDesc,&choice,sizeof(int));
	Opertn_train(scktDesc,choice);
		
	printf("\n Enter the train number you want to delete: ");
	scanf("%d",&tid);
	
	write(scktDesc,&tid,sizeof(tid));
	
	read(scktDesc,&valid,sizeof(valid));
	
	if(valid)
		printf("\nTrain deleted successfully\n");
	else
		printf("Train with given id not present\n");
	return valid;
}


int Opertn_train(int scktDesc,int choice){
	int valid = 0;
	
	switch(choice)
	{
	//----------------------------ADD TRAIN---------------------------
	case 1: valid=addTrain(scktDesc);	
		return valid;
	//----------------------------VIEW TRAIN-------------------------
	case 2: valid=viewTrain(scktDesc);	
		return valid;	
	//--------------------------UPDATE TRAIN--------------------------
	case 3: valid=updateTrain(scktDesc);
		return valid;
	//--------------------------DELETE TRAIN---------------------------	
	case 4: valid=deleteTrain(scktDesc);
		return valid;
	}
	
}

//----------------------------------------------------------------------- Operations on user--------------------------------------------------------//

//--------------------------------------------------------------------------ADD USER------------------------------------------
int addUser(int scktDesc)
{
	int acntType,login_id,valid=0;
	char name[LEN],password[LEN];
	printf("\nChoose account type from below \n");
	printf("1. Agent\n2. Customer\n");
	printf("\nEnter The Type Of Account: \n");
	scanf("%d", &acntType);
	
	printf("Enter User Name: ");
	scanf("%s", name);
	
	strcpy(password,getpass("Enter Password: "));
	
	write(scktDesc, &acntType, sizeof(acntType));
	write(scktDesc, &name, sizeof(name));
	write(scktDesc, &password, strlen(password));
	
	read(scktDesc,&valid,sizeof(valid));	
	
	if(valid){
		read(scktDesc,&login_id,sizeof(login_id));
		printf("Your login id is: %d\n", login_id);
	}
	return valid;	
	
	
}
//----------------------------------------------------------------------VIEW USER------------------------------------------------
int viewUser(int scktDesc)
{
	int no_of_users,valid=0,login_id,acntType;
	char uname[LEN];
	read(scktDesc,&no_of_users,sizeof(no_of_users));
	
	printf("\tUser id\tUser name\tAccount type\n");
	while(no_of_users--){
		read(scktDesc,&login_id,sizeof(login_id));
		read(scktDesc,&uname,sizeof(uname));
		read(scktDesc,&acntType,sizeof(acntType));
		
		if(strcmp(uname, "deleted")!=0 && acntType!=0)
			printf("\t%d\t\t%s\t\t%d\n",login_id,uname,acntType);
	}
	return valid;	
}
//---------------------------------------------------------------------UPDATE USER-------------------------------------------------
int updateUser(int scktDesc)
{
	int choice=2,valid=0,uid;
	char name[LEN],password[LEN];
	write(scktDesc,&choice,sizeof(int));
	Opertn_user(scktDesc,choice);
	printf("\n Enter the User id you want to modify: ");
	scanf("%d",&uid);
	write(scktDesc,&uid,sizeof(uid));
	
	printf("\n1. User Name\n2. Password\n");
	printf("Enter Your Choice: ");
	scanf("%d",&choice);
	write(scktDesc,&choice,sizeof(choice));
	
	if(choice==1){
		read(scktDesc,&name,sizeof(name));
		printf("\n Current name: %s",name);
		printf("\n Updated name:");
		scanf("%s",name);
		write(scktDesc,&name,sizeof(name));
		read(scktDesc,&valid,sizeof(valid));
	}
	else if(choice==2){
		printf("\n Enter Current password: ");
		scanf("%s",password);
		write(scktDesc,&password,sizeof(password));
		read(scktDesc,&valid,sizeof(valid));
		if(valid){
			printf("\n Enter new password:");
			scanf("%s",password);
		}
		else
			printf("\nIncorrect password\n");
		
		write(scktDesc,&password,sizeof(password));
	}
	if(valid){
		read(scktDesc,&valid,sizeof(valid));
		if(valid)
			printf("\n User data updated successfully\n");
	}
	
	return valid;
}
//-----------------------------------------------------------------DELETE USER--------------------------------------
int deleteUser(int scktDesc)
{
	int choice=2,uid,valid=0;
	write(scktDesc,&choice,sizeof(int));
	
	Opertn_user(scktDesc,choice);
	
	printf("\n Enter the id you want to delete: ");
	scanf("%d",&uid);
	write(scktDesc,&uid,sizeof(uid));
	read(scktDesc,&valid,sizeof(valid));
	if(valid)
		printf("\n User deleted successfully\n");
	else
	printf("\nUser with given id not present\n");
	return valid;
}

int Opertn_user(int scktDesc,int choice){
	int valid = 0;
	
	switch(choice)
	{
	//------------------ADD USER------------
	case 1: valid=addUser(scktDesc);
		return valid;	
	//------------------VIEW USER-----------
	case 2: valid=viewUser(scktDesc);
		return valid;
	//------------------UPDATE USER----------
	case 3: valid=updateUser(scktDesc);
		return valid;
	//-----------------DELETE USER----------
	case 4: valid=deleteUser(scktDesc);
		return valid;
	}
}

//-------------------------------------------------------------------------- User function to book tickets ----------------------------------------------------//

//----------------------------------------------------------------------------BOOK TICKET----------------------------------------------------

int bookTickets(int scktDesc)
{
	int view=2,tid,seats;
	int valid=0;
	write(scktDesc,&view,sizeof(int));
	Opertn_train(scktDesc,view);
	printf("\nEnter the train number you want to book: ");
	scanf("%d",&tid);
	write(scktDesc,&tid,sizeof(tid));
			
	printf("\nEnter the no. of seats you want to book: ");
	scanf("%d",&seats);
	write(scktDesc,&seats,sizeof(seats));

	read(scktDesc,&valid,sizeof(valid));
	if(valid)
		printf("\nTicket booked successfully.\n");
	else
		printf("\nSeats were not available.\n");
	return valid;
}
//----------------------------------------------------------------------------VIEW BOOKINGS---------------------------------------------------------------

int viewBookings(int scktDesc)
{
	int no_of_bookings;
	int id,tid,seats;
	int valid=0;
	read(scktDesc,&no_of_bookings,sizeof(no_of_bookings));

	printf("\n\tBooking id\tTicket no\tSeats\n");
	while(no_of_bookings--){
		read(scktDesc,&id,sizeof(id));
		read(scktDesc,&tid,sizeof(tid));
		read(scktDesc,&seats,sizeof(seats));
		
		if(seats!=0)
			printf("\t%d\t\t%d\t\t%d\n",id,tid,seats);
	}

	return valid;
}
//--------------------------------------------------------------------------------------UPDATE BOOKING------------------------------------------------------------
int updateBooking(int scktDesc)
{
	
	int choice = 2,bid,val,valid;
	user_function(scktDesc,choice);
	printf("\n Enter the Booking id you want to modify: ");
	scanf("%d",&bid);
	write(scktDesc,&bid,sizeof(bid));
	printf("Choose from below\n");
	printf("\n1. Increase number of seats\n2. Decrease number of seats\n");
	printf("Enter Your Choice: ");
	scanf("%d",&choice);
	write(scktDesc,&choice,sizeof(choice));

	if(choice==1){
		printf("Enter number of tickets to increase");
		scanf("%d",&val);
		write(scktDesc,&val,sizeof(val));
	}
	else if(choice==2){
		printf("\nEnter number of tickets to decrease");
		scanf("%d",&val);
		write(scktDesc,&val,sizeof(val));
	}
	read(scktDesc,&valid,sizeof(valid));
	if(valid)
		printf("\nBooking updated successfully.\n");
	else
		printf("\nUpdation failed. No more seats available.\n");
	return valid;
}
//-----------------------------------------------------------------------------CANCEL BOOKING--------------------------------------
int cancelBooking(int scktDesc)
{
	int choice = 2,bid,valid;
	user_function(scktDesc,choice);
	printf("\n Enter the Booking id you want to cancel: ");
	scanf("%d",&bid);
	write(scktDesc,&bid,sizeof(bid));
	read(scktDesc,&valid,sizeof(valid));
	if(valid)
		printf("\nBooking cancelled successfully.\n");
	else
		printf("\nCancellation failed.\n");
	return valid;
}
int user_function(int scktDesc,int choice){
	int valid =0;
	switch(choice)
	{
	//-------------BOOK TICKET-----------------
	case 1: valid=bookTickets(scktDesc);
		return valid;
		
	//-------------VIEW BOOKINGS--------------
	case 2: valid=viewBookings(scktDesc);
		return valid;
	
	//-------------UPDATE BOOKING-------------
	case 3: valid=updateBooking(scktDesc);
		return valid;
	
	//-------------CANCEL BOOKING------------
	case 4: valid=cancelBooking(scktDesc);
		return valid;
		
	//-------------LOGOUT------------------
	case 5: return -1;

	}
		
								
}
