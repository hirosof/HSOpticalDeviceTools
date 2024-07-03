#pragma once


#define BEGIN_HSHASH_NAMESPACE  namespace hirosof{ namespace Hash { 
#define END_HSHASH_NAMESPACE }}

BEGIN_HSHASH_NAMESPACE


enum struct EComputeState {
	Updatable = 0,
	Finalized
};

enum struct EHashValueEndian {
	Little = 0,
	Big
};

END_HSHASH_NAMESPACE