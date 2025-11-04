Following file describes the testing process for driver development week to week:

** TEST PLAN **
		
	WEEK ONE:	
			1.- Insert module
			2.- Remove module
			3.- High resolution timer + Callback
			4.- Queue work + Task execution
			5.- Display data
			
	WEEK TWO: 
			1.- Char device
			2.- WRITE function
			3.- POLL function
			4.- READ function


** WEEK 1 **

---------------------------------------------------------------------------------------------------------------------		
|   Test Name		 : Insert module																				|		
|																													|
|	Test Description : Validate that the module can be load using command "sudo insmod 	nxp_simtemp.ko", validation |
|				       method is to display a message in the kernel with the function probe.  Is necessary open		|
|					   a terminal and use the command "sudo dmesg -w" to validate kernel message.					|
|																													|
|	Coverage:		 : Insert module																				|
---------------------------------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------------------------------
|   Test Name		 : Remove module																				|
|																													|
|	Test Description : Validate that the module can be remove using command "sudo rmmod nxp_simtemp", validation 	|
|				       method is to display a message in the kernel with the function remove. Is necessary open		|
|					   a terminal and use the command "sudo dmesg -w" to validate kernel message.					|
|																													|
|	Coverage:		 : Insert module, Remove module																	|
---------------------------------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------------------------------
|   Test Name		 : Timer callback																				|
|																													|
|	Test Description : After loading the module timer callback will display a message in the kernel each second to  |
|					   validate that is working according to the time set. Is necessary open a terminal and use the	|
|					   command "sudo dmesg -w" to validate kernel message											|
|																													|
|	Coverage:		 : Insert module, High resolution timer + Callback												|
---------------------------------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------------------------------
|   Test Name		 : Queue task 																					|
|																													|
|	Test Description : After loading the module timer callback will add in a queue a task with to display a message |
|					   in the kernel each second to validate that is working timer and queue task. Is necessary open| 
|					   a terminal and use the command "sudo dmesg -w" to validate kernel message					|
|																													|
|	Coverage:		 : Insert module, High resolution timer + Callback, Queue work + Task execution					|
---------------------------------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------------------------------
|   Test Name		 : Simulate temperature 																		|
|																													|
|	Test Description : After loading the module timer callback will add in a queue a task to simulate temperature	|
|					   sensor following values are saved in a variable timestamp, temperature, flag for a new sample| 
|					   and a flag if the temperature is above 45°, that variable will be used to display data like	|
|					   kernel message each second. Is necessary open a terminal and use the command "sudo dmesg -w" |
|					   to validate kernel message																	|
|																													|
|	Coverage:		 : Insert module, High resolution timer, Callback, Queue work, Task execution, Display data		|
---------------------------------------------------------------------------------------------------------------------


** WEEK 2 **

---------------------------------------------------------------------------------------------------------------------		
|   Test Name		 : Char device																					|		
|																													|
|	Test Description : Validate that the module can be load using command "sudo insmod 	nxp_simtemp.ko", validation |
|				       method is to display a message in the kernel with the function probe and command " ls /dev " |
|					   to validate that the char device was added successfully. 									|
|																													|
|	Coverage:		 : Char device																					|
---------------------------------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------------------------------
|   Test Name		 : Write time																					|
|																													|
|	Test Description : Execute the program "interface" select option 2 "ms" and insert 500, validate that kernel	|
|					   message with timestamp, temperature and flags are displayed each 500ms. Is necessary opening	|
|					   a terminal and use the command "sudo dmesg -w" to validate kernel message.					|
|																													|
|	Coverage:		 : Char device, WRITE function																	|
---------------------------------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------------------------------
|   Test Name		 : Poll wait																					|
|																													|
|	Test Description : Execute the program "interface" select option 2 "ms" and insert 500, validate that kernel	|
|					   message with timestamp, temperature and flags are displayed each 500ms. Is necessary opening	|
|					   a terminal and use the command "sudo dmesg -w" to validate kernel message. Also validate that|
|					   in the terminal where the programs was run a message "New sample" will be displayed each		| 
|					   500ms																						|
|																													|
|	Coverage:		 : Char device, WRITE function, POLL function													|
---------------------------------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------------------------------
|   Test Name		 : Read	data																					|
|																													|
|	Test Description : Execute the program "interface" select option 2 "ms" and insert 500, validate that kernel	|
|					   message is not present and message with timestamp, temperature and flags are displayed now in|
|					   terminal when the program run. Validate timestamp, temperature and flags						|
|					   																								|
|																													|
|	Coverage:		 : Char device, WRITE function, POLL function, READ function									|
---------------------------------------------------------------------------------------------------------------------																													|

