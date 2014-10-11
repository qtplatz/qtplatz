
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/idaudit.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/quancompounds.hpp>
// #include <adcontrols/quanresponses.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adinterface/method.hpp>

#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>

#include <adportable/debug.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/xml_serializer.hpp>

#include "test_binary_archive.hpp"
#include "test_xml_archive.hpp"

#include <boost/variant.hpp>
#include <functional>
#include <fstream>

namespace test {

    template<class T> struct target {
        target( const char * name, bool has_binary_archiver, bool has_binary_restore, bool has_xml_archiver, bool has_xml_restore, bool exported_serializers = false )
            : name_( name )
            , has_binary_archiver_( has_binary_archiver )
            , has_binary_restore_( has_binary_restore )
            , has_xml_archiver_( has_xml_archiver )
            , has_xml_restore_( has_xml_restore )
            , exported_serializers_( exported_serializers ) {
        }
        T t_;
        const char * name_;
        bool has_binary_archiver_;
        bool has_binary_restore_;
        bool has_xml_archiver_;
        bool has_xml_restore_;
        bool exported_serializers_;
        operator T& () { return t_; }
        std::function<bool(const T&, const T&)> compare_;
    };

    template<class T> void archive_member_detection( const target<T>& t ) {

        std::cout << "--------------------------\nclass " << t.name_ << std::endl;

        std::cout << "\thas bin archive:\t"
                  << ( adportable::binary::has_archive<T, bool( std::ostream&, const T&)>::value ? "yes" : "no" )
                  << (t.has_binary_archiver_ == adportable::binary::has_archive<T, bool( std::ostream&, const T&)>::value ? "\tPASS" : "\tFAIL")
                  << std::endl;

        std::cout << "\thas bin restore:\t"
                  << ( adportable::binary::has_restore<T, bool( std::istream&, T& )>::value ? "yes" : "no" )
                  << (t.has_binary_restore_ == adportable::binary::has_restore<T, bool( std::istream&, T& )>::value ? "\tPASS" : "\tFAIL")
                  << std::endl;
        
        std::cout << "\thas xml archive:\t"
                  << ( adportable::xml::has_archive<T, bool( std::wostream&, const T& )>::value ? "yes" : "no" )
                  << (t.has_xml_archiver_ == adportable::xml::has_archive<T, bool( std::wostream&, const T& )>::value ? "\tPASS" : "\tFAIL")
                  << std::endl;

        std::cout << "\thas xml restore:\t"
                  << ( adportable::xml::has_restore<T, bool( std::wistream&, T& )>::value ? "yes" : "no" )
                  << (t.has_xml_restore_ == adportable::xml::has_restore<T, bool( std::wistream&, T& )>::value ? "\tPASS" : "\tFAIL")
                  << std::endl;
    }
    //

    template<class T, class A> bool read_after_write( const target<T>& t ) {
        boost::filesystem::path path( t.name_ );
        path.replace_extension( A::file_extension() );
        boost::filesystem::remove( path );
        
        std::cout << std::endl;
        std::cout << "\n" << A::title() << " \"" << t.name_ << "\" " << std::flush;
        A ar;
        if ( ar.save( t.t_, path.string().c_str() ) ) {
            std::cout << "\twrite ok." << std::flush;
            T r;
            if ( ar.load( r, path.string().c_str() ) ) {
                std::cout << "\tread ok." << std::flush;
                if ( t.compare_ ) {
                    if ( t.compare_( t.t_, r ) ) {
                        std::cout << "\tPASS." << std::endl;
                        return true;
                    } 
                    else
                        std::cout << "\tFAIL." << std::endl;
                }
                else {
                    std::cout << "\tN/A" << std::endl;
                }
            } else
                std::cout << "\tREAD FAILED." << std::endl;
        }
        else {
            std::cout << "\tWRITE FAILD." << std::endl;
        }
        return false;
    }
    //
    template<class T, class A> bool read( T& r, const char * name ) {
        boost::filesystem::path path( name );
        path.replace_extension( A::file_extension() );

        if ( boost::filesystem::exists( path ) ) {
            std::cout << "\nreading " << A::title() << " \"" << name << "\" ..." << std::flush;            
            A ar;
            if ( ar.load( r, path.string().c_str() ) ) {
                std::cout << "\tread (ok)." << std::flush;
                return true;
            } else {
                std::cout << "\tread failed.";
            }
        }
        return false;
    }

    template<class T, class A> bool write( const T& r, const char * name ) {
        boost::filesystem::path path( name );
        path.replace_extension( A::file_extension() );

        if ( boost::filesystem::exists( path ) )
            boost::filesystem::remove( path );
        std::cout << "\nwriting " << A::title() << " \"" << name << "\" ..." << std::flush;
        A ar;
        if ( ar.save( r, path.string().c_str() ) ) {
            std::cout << "\twrote (ok)." << std::flush;
            return true;
        }
        std::cout << "\twrite failed.";
        return false;
    }

}

adcontrols::MSReferences&
make_msreferences( adcontrols::MSReferences& refs )
{
    using namespace adcontrols;
    refs << MSReference( L"H2O(C2H4)2", true, L"+H" )
         << MSReference( L"H2O(C2H4)3", true, L"+H" )
         << MSReference( L"H2O(C2H4)4", true, L"+H" )
         << MSReference( L"H2O(C2H4)5", true, L"+H" );
    return refs;
}

void
make_process_method( adcontrols::ProcessMethod& m )
{
    using namespace adcontrols;

    MSReferences refs;
    make_msreferences( refs );

    m << CentroidMethod();
    m << ElementalCompositionMethod();
    m << IsotopeMethod();

    auto m3 = MSCalibrateMethod();
    m3.references( refs );
    m << m3;

    m << MSChromatogramMethod();
    m << MSLockMethod();
    m << PeakMethod();
    m << QuanCompounds();
    m << QuanMethod();
    m << TargetingMethod();
    m << CentroidMethod();
}

void
test_process_method()
{
    using namespace adcontrols;

    typedef adcontrols::ProcessMethod T;
    test::target<T> t( "ProcessMethod", true, true, true, true ); // expose binary, xml

    test::archive_member_detection( t );

    t.compare_ = []( const T& a, const T& b ){
        if ( a.size() == b.size() && a.ident().uuid() == b.ident().uuid() ) {
            for ( int i = 0; i < int( a.size() ); ++i ) {
                if ( typeid(a[i]) != typeid(b[i]) )
                    return false;
            }
            return true;
        }   
        return false;
    };
    T ref;
    make_process_method( ref );

    t.t_ = ref;

    // XML test
    test::read_after_write<T, test::xml::archive>( t );
    // Binary test
    test::read_after_write<T, test::binary::archive>( t );

    // read binary write xml -- for visual verification
    //T r;
    //if ( test::read<T, test::binary::archive>( r, t.name_ ) )
    //    test::write<T, test::xml::archive>( t, t.name_ );
}

void
test_process_method1()
{
    using namespace adcontrols;

    typedef adcontrols::ProcessMethod T;
    T failed;

    T ref;
    make_process_method( ref ); // reference method
    
    for ( int n = 0; n < 30; ++n ) {
        for ( auto& m : ref ) {
            test::target<T> t( "ProcessMethod1", true, true, true, true ); // expose binary, xml

            t.compare_ = [] ( const T& a, const T& b ){
                if ( a.size() == b.size() && a.ident().uuid() == b.ident().uuid() ) {
                    for ( int i = 0; i < int( a.size() ); ++i ) {
                        if ( typeid(a[ i ]) != typeid(b[ i ]) )
                            return false;
                    }
                    return true;
                }
                return false;
            };
            t.t_ << m;
            bool result = test::read_after_write<T, test::binary::archive>( t );
            std::cout << "class = " << m.which() << (result ? " success " : " fail ") << m.type().name() << std::endl;
            if ( !result )
                failed << m;
        }
    }
    std::cout << "\nSummary of failed class:";
    for ( auto& m : failed )
        std::cout << "\n\t" << m.type().name();
}

void
test_pm_quancompounds()
{
    using namespace adcontrols;

    typedef adcontrols::ProcessMethod T;

    test::target<T> t( "process_method_quancompounds", true, true, true, true ); // expose binary, xml

    t.compare_ = [] ( const T& a, const T& b ){
        if ( a.size() == b.size() && a.ident().uuid() == b.ident().uuid() ) {
            for ( int i = 0; i < int( a.size() ); ++i ) {
                if ( typeid(a[ i ]) != typeid(b[ i ]) )
                    return false;
            }
            return true;
        }
        return false;
    };
    auto m = QuanCompounds();
    auto c = QuanCompound();
    c.formula( "CH3CN" );
    m << c;
    t.t_ << m;

    //test::read_after_write<T, test::binary::archive>( t );
    test::write<T, test::binary::archive>( t.t_, t.name_ );

    T r;
    test::read<T, test::binary::archive>( r, t.name_ );
}

void 
test_idaudit()
{
    using namespace adcontrols;
    
    typedef idAudit T;
    test::target<idAudit> t( "idAudit", false, false, true, true ); // expose xml
                 
    test::archive_member_detection( t );
    t.compare_ = [] ( const T& a, const T& b )->bool{ return a.uuid() == b.uuid(); };
    test::read_after_write<T, test::xml::archive>( t );
}

void
test_descriptions()
{
    using namespace adcontrols;

    typedef descriptions T;
    test::target<T> t( "descriptions", false, false, true, true ); // expose xml
    
    test::archive_member_detection( t );
    
    // prepare values for test
    t.t_ << adcontrols::description( L"key1", L"value1" );
    t.t_ << adcontrols::description( L"key2", L"value2" );
    t.t_ << adcontrols::description( L"key3", L"value3" );
    
    // test::serializer_test<T, test::xml::archive>(static_cast<const char *>(t.name_))(t, t.name_,
    t.compare_ = [] ( const T& a, const T& b ){
        if ( a.size() == b.size() ) {
            auto ib = b.begin();
            for ( auto ia = a.begin(); ia != a.end(); ia++, ib++ ) {
                if ( (std::wcscmp( ia->key(), ib->key() ) != 0) || (std::wcscmp( ia->text(), ib->text() ) != 0) )
                    return false;
            }
            return true;
        }
        return false;
    };
    test::read_after_write<T, test::xml::archive>( t );
}

void
test_mscalibrate_result()
{
    using namespace adcontrols;
    typedef adcontrols::MSCalibrateResult T;
    test::target<T> t( "MSCalibrateResult", true, true, true, true ); // expose binary, xml

    MSReferences refs;
    t.t_.references( make_msreferences( refs ) );

    std::vector<double> coeffs = { 1, 2, 3, 4 };
    t.t_.calibration( MSCalibration( coeffs ) );
    t.t_.tolerance( 2 );
    t.t_.threshold( 888 );
    t.t_.description( L"ABCDEFG" );
    t.t_.mode( 999 );
    MSAssignedMass a( 5, 1, 128, L"H2O(C2H4)2+H", 999.99, 20, 999.98, true, 0, 1 );
    t.t_.assignedMasses() << a;

    test::archive_member_detection( t );
    t.compare_ = []( const T& a, const T& b ){ return a.mode() == b.mode(); };

    T r;
    test::read_after_write<T, test::binary::archive>( t );
    test::read_after_write<T, test::xml::archive>( t );
    
    // read binary write xml -- for visual verification
    // if ( test::read<T, test::binary::archive>( r, t.name_ ) )
    //     test::write<T, test::xml::archive>( t, t.name_ );
}

void
test_quansequence()
{
    using namespace adcontrols;
    typedef adcontrols::QuanSequence T;
    test::target<T> t( "QuanSequence", true, true, true, true ); // expose binary, xml

    test::archive_member_detection( t );

    for ( int i = 0; i < 10; ++i )
        t.t_ << QuanSample();

    t.compare_ = []( const T& a, const T& b ){ return a.size() == b.size(); };

    test::read_after_write<T, test::xml::archive>( t );
}

void
test_quancompounds()
{
    using namespace adcontrols;

    typedef adcontrols::QuanCompounds T;
    test::target<T> t( "QuanCompounds", false, false, true, true ); // expose xml

    test::archive_member_detection( t );

    t.compare_ = []( const T& a, const T& b ){ return a.ident().uuid() == b.ident().uuid(); };
    
    test::read_after_write<T, test::xml::archive>( t );
}

void
test_adinterface_method()
{
    using namespace adcontrols;

    typedef adinterface::Method T;
    test::target<T> t( "Method", false, false, false, false, true );

    test::archive_member_detection( t );
    //test::read_after_write<T, test::xml::archive>( t );
    //test::read_after_write<T, test::binary::archive>( t );
}

void
test_chromatogram()
{
    using namespace adcontrols;

    typedef adcontrols::Chromatogram T;
    test::target<T> t( "Chromatogram", true, true, false, false );

    test::archive_member_detection( t );

    //test::read_after_write<T, test::xml::archive>( t );
    test::read_after_write<T, test::binary::archive>( t );
}

int
main()
{
    using namespace adcontrols;

    //test_pm_quancompounds();

    test_idaudit();
    test_descriptions();
    // test_process_method1();
    test_process_method();
    test_mscalibrate_result();
    test_quansequence();
    test_quancompounds();
    test_adinterface_method();
    test_chromatogram();

}
