/*
|Simple C++ DEBUG helper|
==================
Author: Gustavo Yudi Bientinezi Matsuzake (Helped by the internet)

Compile with -DDEBUG or define a DEBUG macro (#define DEBUG)
*/

#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <sstream>
#include <iostream>

using namespace std;

/*	MACROS	*/
/*
COLORS
=============================
Implements some MACROS colors
and MACROS flags for fabulous
colorized DEBUG messages */

//colors
#define _NONE	"\033[0m"
#define _GRAY	"\033[38;5;247m"
#define _RED	"\033[1;31m"
#define _GREEN	"\033[1;32m"
#define _YELLOW	"\033[1;33m"
#define _BLUE	"\033[38;5;26m"

//functions
#ifdef DEBUG_COLOR_ENABLE
	#define GRAY(X)		_GRAY << X << _NONE
	#define RED(X)		_RED << X << _NONE
	#define GREEN(X)	_GREEN << X << _NONE
	#define YELLOW(X)	_YELLOW << X << _NONE
	#define BLUE(X)		_BLUE << X << _NONE
#else
	#define GRAY(X) X 
	#define RED(X) X
	#define GREEN(X) X
	#define YELLOW(X) X
	#define BLUE(X) X
#endif

/*STRINGIFY*/
#define STR(X) #X


/* MACRO FUNCTION TO_STRING (X)
=============================
To do something like that:

DEBUG("i = " << i);

*/
#define TO_STRING( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

/*
MACRO FUNCTION DEBUG(X)
=======================
call debug(string func, string msg) function, where:
func - name function
msg  - debug message

This MACRO calls the debug() with the macro __PRETTY_FUNCTION__ that returns the name of the function.
*/

#ifdef NDEBUG
	#if NDEBUG < 0
		#define DEBUG(X, Y) debug(__PRETTY_FUNCTION__, TO_STRING(X), Y)
	#else
		#define DEBUG(X, Y) if(Y <= NDEBUG) debug(__PRETTY_FUNCTION__, TO_STRING(X), Y)
	#endif
#else
	/*if the MACRO DEBUG isn't defined the DEBUG(X) MACRO
	  will not produce any code*/
	#define DEBUG(X, Y)
#endif

/*	FUNCTIONS 	*/

void inline debug(string f, string msg, unsigned int n){
	//prefix
	#ifdef DEBUG_PREFIX_ENABLE
		cout << RED("DEBUG")":";
	#endif
	
	//depth
	cout << "(" << YELLOW(n) << ")";

	//indent
	for(unsigned int i=0; i<(n*2); i++) cout << ' ';

	//function name and message
	cout << BLUE(f) << ": " << GRAY(msg) << endl;
}

//undef colors
#undef NONE	
#undef GRAY	
#undef RED
#undef GREEN	
#undef YELLOW	
#undef BLUE	
#undef _NONE	
#undef _GRAY	
#undef _RED
#undef _GREEN	
#undef _YELLOW	
#undef _BLUE	

//undef strigify
#undef STR

#endif //_DEBUG_H_
