#include<iostream>
#include<stdfloat>
#include<cstdint>
#include<vector>
#include<chrono>
#include <cstdlib>
#ifdef __AVX__
#include <immintrin.h>
#else
#warning AVX is not available. Code will not compile!
#endif
#include <sys/syscall.h>
#include <unistd.h>

#define MAX 1024
#define MAX_ROWS 16
#define MAX_COLS 64
#define STRIDE 64
#define ARCH_GET_XCOMP_PERM     0x1022
#define ARCH_REQ_XCOMP_PERM     0x1023
#define XFEATURE_XTILECFG       17
#define XFEATURE_XTILEDATA      18
using namespace std;

template <typename S>
ostream& operator<<(ostream& os,
		const vector<S>& vector)
{
	// Printing all the elements
	// using <<
	int i = 0;
	int column_size = 32;
	if (vector.size() == 256) {
		column_size = 16;
	}
	for (auto element : vector) {
		if (i > 0 && i % column_size == 0) {
			cout << endl;
		}
		os << element << " ";
		i++;
	}
	return os;
}

//Define tile config data structure 
typedef struct __tile_config
{
	uint8_t palette_id;
	uint8_t start_row;
	uint8_t reserved_0[14];
	uint16_t colsb[16]; 
	uint8_t rows[16]; 
} __tilecfg;

/* Set_tiledata_use() - Invoke syscall to set ARCH_SET_STATE_USE */
static bool set_tiledata_use()
{
	if (syscall(SYS_arch_prctl, ARCH_REQ_XCOMP_PERM, XFEATURE_XTILEDATA)) 
	{
		printf("\n Fail to do XFEATURE_XTILEDATA \n\n");
		return false;
	}
	else
	{
		printf("\n TILE DATA USE SET - OK \n\n");
		return true;
	}

	return true;
}


/* Initialize tile config */
static void init_tile_config (__tilecfg *tileinfo)
{
	int i;
	tileinfo->palette_id = 1;
	tileinfo->start_row = 0;

	for (i = 0; i < 1; ++i)
	{
		tileinfo->colsb[i] = MAX_ROWS;
		tileinfo->rows[i] =  MAX_ROWS;
	}

	for (i = 1; i < 4; ++i)
	{
		tileinfo->colsb[i] = MAX_COLS;
		tileinfo->rows[i] =  MAX_ROWS;
	}

	_tile_loadconfig(tileinfo);
}

int main(){
	__tilecfg tile_data = {0};
	vector<bfloat16_t> src1(16 * 32, 1.0bf16);
	vector<bfloat16_t> src2(16 * 32, 1.0bf16);
	vector<float32_t> res(16 * 16, 0.0f32);

	cout << src1 << endl;
	cout << "===========" << endl;
	cout << src2 << endl;


	// Request permission to linux kernel to run AMX 
	if (!set_tiledata_use())
		exit(-1);
	auto start = std::chrono::high_resolution_clock::now();

	auto iterations = 100000;

	for (int i = 0; i < iterations; i++) {
		// Load tile configuration 
		init_tile_config (&tile_data);
		// Load tile data
		_tile_loadd(1, res.data(), STRIDE);
		_tile_loadd(2, src1.data(), STRIDE);
		_tile_loadd(3, src2.data(), STRIDE);
		// Compute dot-product of bytes in tiles 
		_tile_dpbf16ps (1, 2, 3);
		// Store the tile data to memory
		_tile_stored (1, res.data(), STRIDE);
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

	cout << "Time taken by function: "
		<< duration.count()/iterations << " nanoseconds" << endl;
	cout << "===========" << endl;
	cout << res << endl;
	// Release the tile configuration to return to the init state, 
	// which releases all storage it currently holds
	_tile_release ();
}
