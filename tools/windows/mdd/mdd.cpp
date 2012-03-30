// mdd.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iomanip>
#include <psapi.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <boost/tokenizer.hpp>
#include <boost/filesystem/path.hpp>

#pragma comment(lib, "psapi.lib")

std::string __outfile;

bool __diff = false;
bool __verbose = false;
bool __longform = false;
bool __dump = false;
int __dumpType = 'x';
size_t __dumpLimit = 1024;
size_t __count = 1;
size_t __interval = 6;

class CProcesses {
public:
    typedef std::map<DWORD, std::wstring> vector_type;

private:
    vector_type processes_;
    size_t cProcesses_;
public:
    CProcesses() : cProcesses_(0) {}
    
    void enumProcesses();
    std::vector<DWORD> findProcess( const std::wstring& name );
    inline vector_type::iterator begin() { return processes_.begin(); }
    inline vector_type::iterator end() { return processes_.begin(); }
    inline vector_type::const_iterator begin() const { return processes_.begin(); }
    inline vector_type::const_iterator end() const { return processes_.begin(); }

    static bool getProcessName( DWORD procID, std::wstring& );
    static bool getProcessFilename( DWORD procID, std::wstring& );
    static void printProcessNameAndID( DWORD procID );
};

void
CProcesses::enumProcesses()
{
    DWORD aProcesses[1024];

    DWORD cbNeeded;
    if ( !EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded) )
        return;
    cProcesses_ = cbNeeded / sizeof(DWORD);
    for ( size_t i = 0; i < cProcesses_; ++i ) {
        if ( aProcesses[i] != 0 ) {
            getProcessName( aProcesses[i], processes_[ aProcesses[i] ] );
        }
    }
}

bool
CProcesses::getProcessName( DWORD procID, std::wstring& name )
{
    wchar_t szProcessName[ MAX_PATH ] = L"<unknown>";

    HANDLE hProc = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procID );
    if ( hProc ) {
        HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( hProc, &hMod, sizeof(hMod), &cbNeeded ) )  {
            GetModuleBaseName( hProc, hMod, szProcessName, sizeof(szProcessName)/sizeof(wchar_t) );
            name = szProcessName;
            return true;
        }

    }
    return false;
}

bool
CProcesses::getProcessFilename( DWORD procID, std::wstring& name )
{
    wchar_t szProcessName[ MAX_PATH ] = L"<unknown>";

    HANDLE hProc = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procID );
    if ( hProc ) {
        HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( hProc, &hMod, sizeof(hMod), &cbNeeded ) )  {
			GetModuleFileNameEx( hProc, hMod, szProcessName, MAX_PATH );
            name = szProcessName;
            return true;
        }

    }
    return false;
}

void
CProcesses::printProcessNameAndID( DWORD procID )
{
    wchar_t szProcessName[ MAX_PATH ] = L"<unknown>";

    HANDLE hProc = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procID );
    if ( hProc ) {
        HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( hProc, &hMod, sizeof(hMod), &cbNeeded ) ) 
            GetModuleBaseName( hProc, hMod, szProcessName, sizeof(szProcessName)/sizeof(wchar_t) );

    }
	std::wcout << L"PID: " << procID << L"\t" << szProcessName << std::endl;
}

std::vector<DWORD>
CProcesses::findProcess( const std::wstring& name )
{
    if ( cProcesses_ == 0 )
         enumProcesses();

    std::vector<DWORD> procs;
    for ( vector_type::const_iterator it = processes_.begin(); it != processes_.end(); ++it ) {
        if ( it->second == name )
            procs.push_back( it->first );
    }
    return procs;
}


///////////////////////////////////////////////////////////////////////////////////////////
class CProcessInfo {
    DWORD procId_;
    std::wstring procName_;
    SYSTEM_INFO sysInfo_;
    DWORD mask_;
public:
    ~CProcessInfo();
    CProcessInfo(DWORD procId = 0);

    bool setProcID(DWORD procId);
    DWORD getProcID() const { return procId_; }
    const std::wstring getProcName() const { return procName_; }

    bool VirtualWalk(HANDLE hProc, std::ofstream&, std::vector<class CPageInfo>&);
    bool MemoryWalk(HANDLE hProc, std::ofstream&);
        
    static const char * getMemState(int);
    static const char * getMemProtection(int);
    static const char * getMemType(int);
};

CProcessInfo::~CProcessInfo()
{
    setProcID(0);
}

CProcessInfo::CProcessInfo(DWORD procId) : procId_( procId ), mask_(0)
{
	setProcID( procId );
}

bool
CProcessInfo::setProcID(DWORD procId)
{
    if ( procId == 0 )
        return false;

    procId_ = procId;
    if ( ! CProcesses::getProcessName( procId_, procName_ ) )
        procId_ = 0;

    if ( procId_ ) {
        GetSystemInfo( &sysInfo_ );
        mask_ = sysInfo_.dwAllocationGranularity - 1;
        return true;
    }
    return false;
}

const char *
CProcessInfo::getMemState(int state)
{
    switch( state ) {
    case MEM_COMMIT:  return "COMMIT ";
    case MEM_RESERVE: return "RESERVE";
    case MEM_FREE:    return "FREE   ";
    }
    return "MEM_UNK";
}

const char * 
CProcessInfo::getMemProtection(int protection)
{
    switch( protection ) {
    case PAGE_NOACCESS:            return "NOACCESS  ";
    case PAGE_READONLY:            return "READONLY  ";
    case PAGE_READWRITE:           return "READWRITE ";
    case PAGE_READWRITE|PAGE_GUARD:return "RW|GUARD  ";
    case PAGE_WRITECOPY:           return "WRITECOPY ";
    case PAGE_EXECUTE:             return "EXECUTE   ";
    case PAGE_EXECUTE_READ:        return "EXECUTE_RO";
    case PAGE_EXECUTE_READWRITE:   return "EXECUTE_RW";
    case PAGE_EXECUTE_WRITECOPY:   return "EXECUTE_RW";
    }
    return "          ";
}

const char *
CProcessInfo::getMemType(int type)
{
    switch( type ) {
    case MEM_IMAGE:   return "IMAGE";
    case MEM_MAPPED:  return "MAPPED";
    case MEM_PRIVATE: return "PRIVATE";
    }
    return "";
}

template<class T> static void
dump(const void * p, const void * pBaseAddr, size_t nBytes)
{
    size_t nSize = nBytes / sizeof(T);
    const T * pMem = reinterpret_cast<const T *>(p);
    const T * pMemBase = reinterpret_cast<const T *>(pBaseAddr);

    for ( int i = 0; i < int(nSize); i += 32 ) {

        std::cout << std::endl << std::hex << std::setw(8) << std::setfill(' ') << pMemBase + i << "\t";

        for ( int k = 0; k < (32 / sizeof(T) ); ++k )
            std::cout << std::hex << std::setw( sizeof(T) * 2 ) << pMem[i + k] << " ";

        std::cout << "\t";
        for ( int k = 0; k < 32; ++k ) {
            int c = reinterpret_cast<const unsigned char *>(pMem)[ (i * sizeof(T)) + k];
            putchar( isprint(c) ? c : '.' );
        }

        if ( __dumpLimit && __dumpLimit < size_t(i) ) {
            std::cout << " ==cont==";
            break;
        }
    }
	std::cout << std::endl;
}

class CPageInfo {
public:
    MEMORY_BASIC_INFORMATION mbi_;
    MEMORY_BASIC_INFORMATION prevmbi_;
    
    unsigned long tic_;
    CPageInfo() : tic_(0){
        memset(&mbi_, 0, sizeof(mbi_));
        memset(&prevmbi_, 0, sizeof(prevmbi_));
    }
    CPageInfo(MEMORY_BASIC_INFORMATION& mbi, unsigned long tic) : mbi_(mbi) {
        memset(&prevmbi_, 0, sizeof(prevmbi_));
    }
    CPageInfo(const CPageInfo& t) : mbi_(t.mbi_), prevmbi_(t.prevmbi_), tic_(t.tic_) {
    }
    CPageInfo(const CPageInfo& t, const CPageInfo& prev) : mbi_(t.mbi_), prevmbi_(prev.mbi_), tic_(t.tic_) {
    }
    bool operator < ( const CPageInfo& t ) {
        void * pThis = mbi_.BaseAddress ? mbi_.BaseAddress : prevmbi_.BaseAddress;
        void * pOther = t.mbi_.BaseAddress ? t.mbi_.BaseAddress : t.prevmbi_.BaseAddress;
        return pThis < pOther;
    }
    bool operator == ( void * baseAddr ) const {
        return mbi_.BaseAddress == baseAddr;
    }
    bool operator == ( const CPageInfo& t ) const {
        return mbi_.BaseAddress == t.mbi_.BaseAddress 
            && mbi_.RegionSize == t.mbi_.RegionSize
            && mbi_.State == t.mbi_.State;
    }
    static void difference( const std::vector<CPageInfo>&, const std::vector<CPageInfo>&, std::vector<CPageInfo>& );
    static void print( const CPageInfo& t );
    static void print_mbi( const MEMORY_BASIC_INFORMATION& mbi ) {
        std::cout << std::hex
            << "0x" << std::setfill('0') << std::setw(8) << mbi.BaseAddress << "\t"
            << "0x" << std::setfill('0') << std::setw(8) << mbi.RegionSize << "\t"
            << CProcessInfo::getMemState( mbi.State ) << "\t"
            << CProcessInfo::getMemProtection( mbi.Protect );
    }
};

std::vector< CPageInfo > __pageInfo;

void
CPageInfo::difference( const std::vector<CPageInfo>& prev, const std::vector<CPageInfo>& curr, std::vector<CPageInfo>& res )
{
    for ( std::vector<CPageInfo>::const_iterator it = curr.begin(); it != curr.end(); ++it ) {
        std::vector<CPageInfo>::const_iterator prevIt = std::find( prev.begin(), prev.end(), it->mbi_.BaseAddress );
        if ( prevIt == prev.end() )
            res.push_back( *it ); // new

        if ( prevIt != prev.end() ) {
            if ( ! (*prevIt == *it) )
                res.push_back( CPageInfo( *it, *prevIt ) );
        }
    }
    for ( std::vector<CPageInfo>::const_iterator prevIt = prev.begin(); prevIt != prev.end(); ++prevIt ) {
        std::vector<CPageInfo>::const_iterator currIt = std::find( curr.begin(), curr.end(), prevIt->mbi_.BaseAddress );
        if ( currIt == curr.end() )
            res.push_back( CPageInfo( CPageInfo(), *prevIt ) ); // deleted
    }
    std::sort( res.begin(), res.end() );
}

void
CPageInfo::print( const CPageInfo& t )
{
    if ( t.mbi_.BaseAddress )
        print_mbi(t.mbi_);
    else
		return; //print_mbi(t.prevmbi_);

    std::cout << "<===\t";

    //size_t nSize = t.mbi_.BaseAddress ? t.mbi_.RegionSize : t.prevmbi_.RegionSize;
    //std::cout << std::dec << std::setfill(' ') << std::setw(6) << std::right << std::showpos << (nSize / 1024) << " K\t";

    if ( t.prevmbi_.BaseAddress == 0 ) {
        std::cout << "\tadded\t";
    } else if ( t.mbi_.BaseAddress == 0 ) {
        std::cout << "\tdeleted";
    } else {
        if ( t.mbi_.RegionSize != t.prevmbi_.RegionSize ) {
            char sign = t.mbi_.RegionSize > t.prevmbi_.RegionSize ? '+' : '-';
            size_t nDiff = sign == '+' ? t.mbi_.RegionSize - t.prevmbi_.RegionSize : t.prevmbi_.RegionSize - t.mbi_.RegionSize;

            char * op = "resize";
            if ( t.mbi_.State == MEM_COMMIT && t.prevmbi_.State == MEM_FREE )
                op = "alloc";
            if ( t.mbi_.State == MEM_FREE && t.prevmbi_.State == MEM_COMMIT )
                op = "free";

            std::cout << "(" << sign << std::dec << (nDiff / 1024) << "K) " << op << " from:\t0x" << std::setw(8) << std::setfill('0') << std::hex << t.prevmbi_.RegionSize << "\t";
        }
        if ( t.mbi_.State != t.prevmbi_.State )
            std::cout << CProcessInfo::getMemState( t.prevmbi_.State ) << "\t";
        if ( t.mbi_.Protect != t.prevmbi_.Protect )
            std::cout << CProcessInfo::getMemProtection( t.prevmbi_.Protect ) << "\t";
    }
    std::cout << std::endl;
}


bool
CProcessInfo::VirtualWalk(HANDLE hProc, std::ofstream& outf, std::vector<CPageInfo>& pageInfo )
{
    MEMORY_BASIC_INFORMATION mbi, mbi2;
    unsigned char * lpMem = 0;

	if ( __verbose ) {
		std::wcout << L"Addr\tSize(bytes)\taccess permition\ttype/module" << std::endl;
		outf << L"Addr\tSize(bytes)\taccess permition\ttype/module" << std::endl;
	}

    size_t cMemCommit(0);
    size_t cMemCommitSize(0);

    size_t cMemReserve(0);
    size_t cMemReserveSize(0);

    size_t cMemFree(0);
    size_t cMemFreeSize(0);

    size_t cMemTotal(0);
    size_t cMemTotalSize(0);

    unsigned long tic = ::GetTickCount();

    while ( lpMem < sysInfo_.lpMaximumApplicationAddress ) {

        if ( VirtualQueryEx( hProc, lpMem, &mbi, sizeof(mbi) ) ) {

            unsigned long long granuOffset = reinterpret_cast<unsigned long long>(static_cast<unsigned char *>(mbi.BaseAddress)) & mask_;
            bool atBoundary = granuOffset == 0;
            if ( mbi.State == MEM_FREE && !atBoundary) { 
                size_t size = sysInfo_.dwAllocationGranularity - static_cast<size_t>(granuOffset);  // no 64bit compatible
                if ( mbi.RegionSize > size )
                    mbi.RegionSize = size;
            }

            unsigned char * lpMem2 = reinterpret_cast<unsigned char *>(mbi.BaseAddress) + mbi.RegionSize;
            SIZE_T n = 1;
            if ( ( lpMem2 >= sysInfo_.lpMaximumApplicationAddress ) 
                || ( VirtualQueryEx( hProc, lpMem2, &mbi2, sizeof(mbi2) ) == 0 ) )
                lpMem2 = 0;

            if ( mbi.State == MEM_COMMIT ) {
                cMemCommit++;
                cMemCommitSize += mbi.RegionSize;
            } else if ( mbi.State == MEM_RESERVE ) {
                cMemReserve++;
                cMemReserveSize += mbi.RegionSize;
            } else if ( mbi.State == MEM_FREE ) {
                cMemFree++;
                cMemFreeSize += mbi.RegionSize;
            }
            cMemTotal++;
            cMemTotalSize += mbi.RegionSize;

            bool bMappedFile = false;
            wchar_t mappedFile[255] = L"";
            memset(mappedFile, 0, sizeof(mappedFile));

            if ( GetMappedFileName(hProc, lpMem, mappedFile, 255) ) {
                bMappedFile = true;
				if ( ! __longform ) {
					std::wstring str(mappedFile);
					std::wstring::size_type pos = str.find_last_of(L"\\");
					if ( pos != std::wstring::npos )
						wcsncpy_s( mappedFile, str.substr(pos + 1).c_str(), sizeof(mappedFile)/sizeof(mappedFile[0]) );
				}
            }
            bool bMemFree = ( mbi.State == MEM_FREE ) && ( mbi.Protect == PAGE_NOACCESS );
            bool bMemPrivate = ( mbi.State == MEM_COMMIT ) && ( mbi.Protect == PAGE_READWRITE) && ( mbi.Type == MEM_PRIVATE );
            bool bMemReserve = ( mbi.State == MEM_RESERVE ) && ( mbi.Type == MEM_PRIVATE );
            size_t nAlloc = 0;
            size_t nFree = 0;
            size_t nGap = lpMem2 ? unsigned long(lpMem2 - lpMem) - mbi.RegionSize : 0;
            if ( bMemPrivate && lpMem2 ) {
                nAlloc = mbi.RegionSize;
                nFree = unsigned long(lpMem2 - lpMem) - nAlloc;
                if ( mbi2.State == MEM_FREE && mbi2.Protect == PAGE_NOACCESS )
                    nFree += mbi2.RegionSize;
            }

            if ( bMemFree || bMemPrivate || bMemReserve )
				pageInfo.push_back( CPageInfo(mbi, tic) );

            if ( __verbose ) {
                CPageInfo::print_mbi( mbi );
                if ( bMappedFile )
					std::cout << "\t" << mappedFile << std::endl;
                else if ( bMemPrivate )
                    std::cout << "\t\t" << std::dec << std::setw(6) << std::setfill(' ') << (nAlloc / 1024) << " K / " << (nFree / 1024) << " K" << std::endl;
                else if ( bMemFree || bMemReserve )
                    std::cout << "\t\t" << std::dec << std::setw(6) << "         / " << (mbi.RegionSize / 1024) << " K" << std::endl;
                else
                    std::cout << "\t" << CProcessInfo::getMemType( mbi.Type ) << std::endl;
            }

            if ( __dump && bMemPrivate && !bMappedFile ) {

                void * p = VirtualAlloc(0, mbi.RegionSize, MEM_COMMIT, PAGE_READWRITE);
                    
                SIZE_T cb = 0;
                if ( ReadProcessMemory(hProc, mbi.BaseAddress, p, mbi.RegionSize, &cb) ) 
                    dump<unsigned short>(p, mbi.AllocationBase, cb);

                VirtualFree(p, 0, MEM_RELEASE);
            }
        }

        lpMem = reinterpret_cast<unsigned char *>(mbi.BaseAddress) + mbi.RegionSize;
    }

    std::cout
        << "\tCommited: 0x" << std::hex << std::setfill('0') << cMemCommitSize  << " (" << std::dec << cMemCommitSize / 1024 << " K)"
        << "\tReserved: 0x" << std::hex << std::setfill('0') << cMemReserveSize << " (" << std::dec << cMemReserveSize / 1024 << " K)"
        << "\tVM Alloced: 0x" << std::hex << std::setfill('0') << cMemCommitSize + cMemReserveSize << "(" << std::dec << (cMemCommitSize + cMemReserveSize) / 1024 << " K)" << std::endl;

    outf
		<< "VM:\t" << std::dec << cMemCommitSize << "\t"
		<< cMemReserveSize << "\t"
		<< (cMemCommitSize + cMemReserveSize) << "\t";

    return true;
}

bool 
CProcessInfo::MemoryWalk(HANDLE hProc, std::ofstream& outf)
{
    PROCESS_MEMORY_COUNTERS mc;
    memset(&mc, 0, sizeof(mc));

    if ( GetProcessMemoryInfo(hProc, &mc, sizeof( mc ) ) ) {
        std::cout << "PROCESS MEMORY COUNTERS" << std::resetiosflags( std::ios::right | std::ios::showbase ) << std::endl
			// << "PageFaultCount            \t0x" << std::hex << mc.PageFaultCount << std::endl
            << "WorkingSetSize            \t" << std::setw(8) << std::hex << mc.WorkingSetSize 
            << "\t(" << std::dec << std::setw(6) << std::setfill(' ') << mc.WorkingSetSize / 1024 << " K)" << std::endl
            << "QuotaPeakPagedPoolUsage   \t" << std::setw(8) << std::hex << mc.QuotaPeakNonPagedPoolUsage 
            << "\t(" << std::dec << std::setw(6) << std::setfill(' ') << mc.QuotaPeakNonPagedPoolUsage / 1024 << " K)" << std::endl
            << "QuotaPagedPoolUsage       \t" << std::setw(8) << std::hex << mc.QuotaPagedPoolUsage
            << "\t(" << std::dec << std::setw(6) << std::setfill(' ') << mc.QuotaPagedPoolUsage / 1024 << " K)" << std::endl
            << "QuotaPeakNonPagedPoolUsage\t" << std::setw(8) << std::hex << mc.QuotaPeakNonPagedPoolUsage
            << "\t(" << std::dec << std::setw(6) << std::setfill(' ') << mc.QuotaPeakNonPagedPoolUsage / 1024 << " K)" << std::endl
            << "QuotaPeakPagedPoolUsage   \t" << std::setw(8) << std::hex << mc.QuotaPeakPagedPoolUsage 
            << "\t(" << std::dec << std::setw(6) << std::setfill(' ') << mc.QuotaPeakPagedPoolUsage / 1024 << " K)" << std::endl
            << "PagefileUsage             \t" << std::setw(8) << std::hex << mc.PagefileUsage << std::endl
            << "PeakPagefileUsage         \t" << std::setw(8) << std::hex << mc.PeakPagefileUsage << std::endl;

		outf << "::\t"
			<< mc.WorkingSetSize  << "\t"
            << mc.QuotaPeakNonPagedPoolUsage  << "\t"
            << mc.QuotaPagedPoolUsage << "\t"
            << mc.QuotaPeakNonPagedPoolUsage << "\t"
            << mc.QuotaPeakPagedPoolUsage  << "\t"
            << mc.PagefileUsage  << "\t"
			<< mc.PeakPagefileUsage  << "\t";

        return true;
    } 
    return false;
}

bool
memory_walk(CProcessInfo& info)
{
    static bool once = true;

    std::ofstream outf(__outfile.c_str(), std::ios::out | std::ios::app );

    char tmstr[256];
    if ( GetDateFormatA(LOCALE_SYSTEM_DEFAULT,  0, 0, 0, tmstr, sizeof(tmstr)/sizeof(tmstr[0])) ) {
        std::cout << tmstr << "\t";
        outf << tmstr << "\t";
    }

    if ( GetTimeFormatA(LOCALE_SYSTEM_DEFAULT,  0, 0, 0, tmstr, sizeof(tmstr)/sizeof(tmstr[0])) ) {
        std::cout << tmstr << std::endl;
        outf << tmstr << "\t";
    }

    HANDLE hProc = OpenProcess( PROCESS_ALL_ACCESS, FALSE, info.getProcID() );
    if ( hProc ) {
		if ( once )
			once = false;

        std::vector<CPageInfo> pageInfo;
        info.VirtualWalk(hProc, outf, pageInfo);
        info.MemoryWalk(hProc, outf);
        outf << std::endl;

        if ( __diff && ! __pageInfo.empty() ) {
            std::vector<CPageInfo> res;
            CPageInfo::difference( __pageInfo, pageInfo, res );
            if ( ! res.empty() )
                std::for_each( res.begin(), res.end(), CPageInfo::print );
        }
        __pageInfo = pageInfo;

        CloseHandle(hProc);
    } else {
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
int
main(int argc, char * argv[])
{
    __outfile = "mdd.log";
    std::wstring procname = L"mdd.exe";

    --argc;
    ++argv;
    while ( argc-- ) {
        if ( ( strcmp("-o", *argv) == 0 ) && argc ) {
            ++argv;
            --argc;
            __outfile = *argv;
        } else if ( ( strcmp( "-p", *argv) == 0 ) && argc ) {
            ++argv;
            --argc;
			std::string s = *argv;
			procname.assign( s.begin(), s.end() );
		} else if ( strcmp( "-l", *argv ) == 0 ) {
            __verbose = true;
            __longform = true;
            std::cerr << "verbose long format mode on" << std::endl;
		} else if ( strcmp( "-v", *argv ) == 0 ) {
            __verbose = true;
            std::cerr << "verbose mode on" << std::endl;
        } else if ( strcmp( "-d", *argv ) == 0 ) {
            __dump = true;
            if ( argc && ( *argv[1] != '-' ) ) {
                ++argv;
                --argc;
                __dumpLimit = atoi( *argv );
            }
            std::cerr << "dump: " << __dumpLimit << " words" << std::endl;
        } else if ( strcmp( "-x", *argv ) == 0 ) {
            __dump = true;
            __dumpType = 'x';  // hex
		} else if ( strcmp( "-c", *argv ) == 0 ) {
            if ( argc && ( *argv[1] != '-' ) ) {
                ++argv;
                --argc;
                __count = atoi( *argv );
            } else {
                __count = (-1); // for ever
            }
            std::cerr << "count: " << __count << " times repeat" << std::endl;
        } else if ( ( strcmp( "-i", *argv ) == 0 ) && argc ) {
            ++argv;
            --argc;
            __interval = atoi( *argv );
            std::cerr << "interval: " << __interval << " seconds" << std::endl;
        } else if ( (strcmp( "-diff", *argv ) == 0 ) || (strcmp( "--diff", *argv ) == 0 ) ) {
            __diff = true;
        } else if ( '-' == **argv ) {
            std::cerr << "mdd [-p process_name] [-v] [-c [count]] [-i interval] [-o logfile]" << std::endl;
            return 0;
        }
		++argv;
    }

    if ( __diff && __count == 1 )
        __count = 2;

    while (__count--) {
        CProcesses processes;

        std::vector<DWORD> plist = processes.findProcess( procname );

        while ( plist.empty() ) {
            for ( CProcesses::vector_type::const_iterator it = processes.begin(); it != processes.end(); ++it )
                CProcesses::printProcessNameAndID( it->first );
            std::wcout << procname << L" is not running.  Waiting for startup..." << std::endl;
            Sleep( 1000 * 6 );
            processes.enumProcesses();
            plist = processes.findProcess( procname );
            // return 0;
        } 
        DWORD procId = plist[0];  // take first one if more than two instance running
   
        CProcessInfo info( procId ); 
        CProcesses::printProcessNameAndID( info.getProcID() );

        std::wstring process_filename;
        CProcesses::getProcessFilename( info.getProcID(), process_filename );

        // _unlink( __outfile.c_str() );

        do {
            std::ofstream outf(__outfile.c_str(), std::ios::out | std::ios::app );
            outf << "DATE\tTIME\t"
                << "VM\tCOMMIT\t"
                << "VM:RESERVE\t"
                << "VM:TOTAL\t" 
                << "PageFaultCount\t"
                << "WorkingSetSize\t"
                << "QuotaPeakPagedPoolUsage\t"
                << "QuotaPagedPoolUsage\t"
                << "QuotaPeakNonPagedPoolUsage\t"
                << "QuotaPeakPagedPoolUsage\t"
                << "PagefileUsage\t"
                << "PeakPagefileUsage\t" << std::endl;
        } while(0);

        memory_walk(info);
        if ( __count )
            Sleep( 1000 * int(__interval) ); // approx 0.1 min interval
    };

}

