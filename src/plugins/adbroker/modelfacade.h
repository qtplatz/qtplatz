#ifndef MODELFACADE_H
#define MODELFACADE_H

#include <boost/variant.hpp>
#include <vector>

class AnalysisModel;
class AcquireModel;
class SequenceModel;
class BatchprocModel;

class ModelFacade
{
public:
    ModelFacade();

    typedef boost::variant< AcquireModel
                          , AnalysisModel
                          > value_type;
    typedef std::vector< value_type > vector_type;

    inline vector_type::iterator begin() { return models_.begin(); }
    inline vector_type::iterator end()   { return models_.end(); }
    inline vector_type::const_iterator begin() const { return models_.begin(); }
    inline vector_type::const_iterator end() const { return models_.end(); }

    template<class T> inline vector_type::iterator find() {
        int n = models_.size();
        for ( vector_type::iterator it = begin(); it != end(); ++it )
            if ( T * p = boost::get<T>(&(*it)) )
                return it;
        return end();
    }

    template<class T> T* getModel() {
        vector_type::iterator it = find<T>();
        if ( it != end() )
            return boost::get<T>( &(*it) );
        return 0;
    }

    template<class T> void setModel(T& t) {
        models_.push_back(t);
    }

private:
    vector_type models_;
};

#endif // MODELFACADE_H
