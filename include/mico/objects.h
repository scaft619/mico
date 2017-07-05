#ifndef MICO_OBJECTS_H
#define MICO_OBJECTS_H

#include <memory>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include "mico/statements.h"
#include "mico/expressions.h"
#include "mico/environment.h"

namespace mico {

namespace objects {

    enum class type {
        NULL_OBJ = 0,
        BOOLEAN,
        INTEGER,
        FLOAT,
        STRING,
        TABLE,
        ARRAY,
        REFERENCE,
        RETURN,
        FUNCTION,
        CONT_CALL,
        BUILTIN,
        ERROR,
    };

    struct name {
        static
        const char *get( type t )
        {
            switch (t) {
            case type::NULL_OBJ   : return "OBJ_NULL";
            case type::BOOLEAN    : return "OBJ_BOOLEAN";
            case type::INTEGER    : return "OBJ_INTEGER";
            case type::FLOAT      : return "OBJ_FLOAT";
            case type::STRING     : return "OBJ_STRING";
            case type::TABLE      : return "OBJ_TABLE";
            case type::ARRAY      : return "OBJ_ARRAY";
            case type::REFERENCE  : return "OBJ_REFERENCE";
            case type::RETURN     : return "OBJ_RETURN";
            case type::FUNCTION   : return "OBJ_FUNCTION";
            case type::BUILTIN    : return "OBJ_BUILTIN";
            case type::CONT_CALL  : return "OBJ_CONT_CALL";
            case type::ERROR      : return "OBJ_ERROR";
            }
            return "<invalid>";
        }
    };

    struct cast {
        template <typename ToT, typename FromT>
        static
        ToT *to( FromT *obj )
        {
            return static_cast<ToT *>(obj);
        }
    };

    struct base {
        virtual ~base( ) = default;
        virtual type get_type( ) const = 0;
        virtual std::string str( ) const = 0;

        virtual std::uint64_t hash( ) const
        {
            std::hash<std::string> h;
            return h( str( ) )  ;
        }

        virtual bool equal( const base *other ) const
        {
            return str( ) == other->str( );
        }

        static
        std::uint64_t hash64(uint64_t x)
        {
            std::hash<std::uint64_t> h;
            return h(x);
        }

        virtual std::size_t size( ) const
        {
            return 0;
        }

        static
        bool is_container( const base *o )
        {
            return (o->get_type( ) == type::ARRAY)
                || (o->get_type( ) == type::TABLE)
                 ;
        }

        virtual void env_reset( )
        { }

        virtual std::shared_ptr<base> clone( ) const = 0;

    };

    template <type TN>
    struct typed_base: public base {
        type get_type( ) const
        {
            return TN;
        }
    };

    using sptr  = std::shared_ptr<base>;
    using uptr  = std::unique_ptr<base>;
    using slist = std::vector<sptr>;
    using ulist = std::vector<uptr>;

    template <type TName>
    struct type2object;

    template <>
    struct type2object<type::FLOAT> {
        using native_type = double;
    };

    template <>
    struct type2object<type::INTEGER> {
        using native_type = std::int64_t;
    };

    template <type>
    class derived;

    template <>
    class derived<type::NULL_OBJ>: public typed_base<type::NULL_OBJ> {
        using this_type = derived<type::NULL_OBJ>;
    public:
        using sptr = std::shared_ptr<this_type>;
        std::string str( ) const
        {
            return "null";
        }
        static
        sptr make( )
        {
            static auto val = std::make_shared<this_type>( );
            return val;
        }
        std::uint64_t hash( ) const override
        {
            return 0;
        }

        bool equal( const base *o ) const override
        {
            return (o->get_type( ) == get_type( ));
        }

        std::shared_ptr<base> clone( ) const override
        {
            return make( );
        }

    };

    template <>
    class derived<type::STRING>: public typed_base<type::STRING> {

        using this_type = derived<type::STRING>;

    public:

        using sptr = std::shared_ptr<this_type>;
        using value_type = std::string;

        std::string str( ) const override
        {
            std::ostringstream oss;
            oss << "\"" << value( ) << "\"";
            return oss.str( );
        }
        derived( value_type val )
            :value_(std::move(val))
        { }
        const value_type &value( ) const
        {
            return value_;
        }
        value_type &value( )
        {
            return value_;
        }

        std::size_t hash( ) const override
        {
            std::hash<std::string> h;
            return h(value_);
        }

        bool equal( const base *other ) const override
        {
            if( other->get_type( ) == get_type( ) ) {
                auto o = static_cast<const this_type *>( other );
                return o->value( ) == value( );
            }
            return false;
        }

        std::shared_ptr<base> clone( ) const override
        {
            return std::make_shared<this_type>( value_ );
        }

    private:
        value_type value_;
    public:
    };

    template <type TN>
    class env_object: public typed_base<TN> {

    public:

        env_object( std::shared_ptr<environment> e )
            :env_(e)
        {
            e->lock( );
        }

        ~env_object( )
        {
            drop( );
        }

        void lock( )
        {
        }

        std::shared_ptr<environment> env( )
        {
            auto l = env_.lock( );
            return l;
        }

        const std::shared_ptr<environment> env( ) const
        {
            auto l = env_.lock( );
            return l;
        }

    private:

        void drop( )
        {
            auto p = env( );
            if( p ) {
                p->unlock( );
                p->drop( );
            }
        }

        std::weak_ptr<environment> env_;
        //std::shared_ptr<environment> env_;
    };

    template <>
    class derived<type::FUNCTION>: public env_object<type::FUNCTION> {
        using this_type = derived<type::FUNCTION>;
    public:
        using sptr = std::shared_ptr<this_type>;

        derived( std::shared_ptr<environment> e,
                  std::shared_ptr<ast::expression_list> par,
                  std::shared_ptr<ast::statement_list> st )
            :env_object(e)
            ,params_(par)
            ,body_(st)
        { }

        std::string str( ) const override
        {
            std::ostringstream oss;
            oss << "fn(" << params_->size( ) << ")";
            return oss.str( );
        }

        ast::expression_list &params( )
        {
            return *params_;
        }

        const ast::expression_list &params( ) const
        {
            return *params_;
        }

        const ast::statement_list &body( ) const
        {
            return *body_;
        }

        ast::statement_list &body( )
        {
            return *body_;
        }

        std::shared_ptr<base> clone( ) const override
        {
            return std::make_shared<this_type>( environment::make( env( ) ),
                                                params_, body_ );
        }

    private:
        std::shared_ptr<ast::expression_list> params_;
        std::shared_ptr<ast::statement_list>  body_;
    };

    template <>
    class derived<type::BUILTIN>: public env_object<type::BUILTIN> {
        using this_type = derived<type::BUILTIN>;
    public:
        using sptr = std::shared_ptr<this_type>;

        derived( std::shared_ptr<environment> e )
            :env_object(e)
        { }

        std::string str( ) const override
        {
            std::ostringstream oss;
            oss << "fn(" << static_cast<const void *>(this) << ")";
            return oss.str( );
        }

        virtual
        objects::sptr call( objects::slist &, environment::sptr )
        {
            return derived<type::NULL_OBJ>::make( );
        }

        virtual
        void init( environment::sptr )
        { }

    private:
    };

    template <>
    class derived<type::CONT_CALL>: public env_object<type::CONT_CALL> {

        using this_type = derived<type::CONT_CALL>;
    public:
        using sptr = std::shared_ptr<this_type>;

        derived(objects::sptr obj, environment::sptr e)
            :env_object(e)
            ,obj_(obj)
        { }

        std::string str( ) const override
        {
            std::ostringstream oss;
            oss << "cc(" << ")";
            return oss.str( );
        }

        objects::sptr  value( )
        {
            return obj_;
        }

        std::shared_ptr<base> clone( ) const override
        {
            return std::make_shared<this_type>( obj_, env( ) );
        }

    private:
        objects::sptr obj_;
    };

    template <>
    class derived<type::RETURN>: public typed_base<type::RETURN> {
        using this_type = derived<type::RETURN>;
    public:
        using sptr = std::shared_ptr<this_type>;

        using value_type = objects::sptr;

        derived( value_type val )
            :value_(val)
        { }

        std::string str( ) const override
        {
            std::ostringstream oss;
            oss << "return " << value( )->str( );
            return oss.str( );
        }

        value_type &value( )
        {
            return value_;
        }

        const value_type &value( ) const
        {
            return value_;
        }

        static
        sptr make( objects::sptr res )
        {
            return std::make_shared<this_type>( res );
        }

        std::shared_ptr<base> clone( ) const override
        {
            return std::make_shared<this_type>( value_ );
        }

    private:
        value_type value_;
    };

    template <>
    class derived<type::REFERENCE>: public typed_base<type::REFERENCE> {
        using this_type = derived<type::REFERENCE>;
    public:

        using sptr = std::shared_ptr<this_type>;
        using value_type = objects::sptr;

        derived(value_type val)
            :value_(val)
        { }

        std::string str( ) const override
        {
            std::ostringstream oss;
            oss << value( )->str( );
            return oss.str( );
        }

        const value_type &value( ) const
        {
            return value_;
        }

        value_type &value( )
        {
            return value_;
        }

        static
        sptr make( value_type val )
        {
            return std::make_shared<this_type>(val);
        }

        std::uint64_t hash( ) const override
        {
            return value_->hash( );
        }

        bool equal( const base *other ) const override
        {
            return value_->equal( other );
        }

        std::shared_ptr<base> clone( ) const override
        {
            return std::make_shared<this_type>( value_->clone( ) );
        }

    private:
        value_type value_;
    };

    template <>
    class derived<type::ARRAY>: public typed_base<type::ARRAY> {
        using this_type = derived<type::ARRAY>;
    public:

        using sptr = std::shared_ptr<this_type>;
        using cont = derived<type::REFERENCE>;
        using cont_sptr = std::shared_ptr<cont>;
        using value_type = std::vector<cont_sptr>;

        std::string str( ) const override
        {
            std::ostringstream oss;
            oss << "[";
            bool first = true;
            for( auto &v: value( ) ) {
                if( first ) {
                    first = false;
                } else {
                    oss << ", ";
                }
                oss << v->str( );
            }
            oss << "]";
            return oss.str( );
        }

        const value_type &value( ) const
        {
            return value_;
        }

        std::size_t size( ) const override
        {
            return value_.size( );
        }

        value_type &value( )
        {
            return value_;
        }

        objects::sptr at( std::int64_t id )
        {
            auto size = static_cast<std::size_t>(id);
            if( (size < value_.size( )) && ( id >= 0) ) {
                return value_[size];
            } else {
                return derived<type::NULL_OBJ>::make( );
            }
        }

        void push( objects::sptr val )
        {
            value_.emplace_back( cont::make(val) );
        }

        static
        sptr make( )
        {
            return std::make_shared<this_type>( );
        }

        std::uint64_t hash( ) const override
        {
            auto init = static_cast<std::uint64_t>(get_type( ));
            std::uint64_t h = base::hash64( init );
            for( auto &o: value( ) ) {
                h = base::hash64( h + o->hash( ) );
            }
            return h;
        }

        bool equal( const base *other ) const override
        {
            if( other->get_type( ) == get_type( ) ) {
                auto o = static_cast<const this_type *>( other );
                if( o->value( ).size( ) == value( ).size( ) ) {
                    std::size_t id = value( ).size( );
                    while( id-- ) {
                        auto other = o->value( )[id]->value( ).get( );
                        if( !value( )[id]->equal( other ) ) {
                            return false;
                        }
                    }
                    return true;
                }
            }
            return false;
        }

        std::shared_ptr<base> clone( ) const override
        {
            auto res = std::make_shared<this_type>( );
            for( auto &v: value_ ) {
                res->push( v->value( )->clone( ) );
            }
            return res;
        }

    private:

        void env_reset( ) override
        {
            for( auto &v: value_ ) {
                //v->value( ).reset( );
                v.reset( );
            }
        }
        value_type  value_;
    };

    template <>
    class derived<type::BOOLEAN>: public typed_base<type::BOOLEAN> {
        using this_type = derived<type::BOOLEAN>;

        struct key { };

    public:

        using sptr = std::shared_ptr<this_type>;

        derived( bool v )
            :value_(v)
        { }

        std::string str( ) const override
        {
            std::ostringstream oss;
            oss << (value_ ? "true" : "false");
            return oss.str( );
        }

        bool value( ) const
        {
            return value_;
        }

        static
        sptr make( bool val )
        {
            static auto true_this  = std::make_shared<this_type>( true );
            static auto false_this = std::make_shared<this_type>( false );
            return val ? true_this : false_this;
        }

        std::uint64_t hash( ) const override
        {
            return value_ ? 1 : 0;
        }

        bool equal( const base *other ) const override
        {
            if( other->get_type( ) == get_type( ) ) {
                auto o = static_cast<const this_type *>( other );
                return o->value( ) == value( );
            }
            return false;
        }

        std::shared_ptr<base> clone( ) const override
        {
            return make( value_ );
        }

    private:

        bool value_;
    };

    template <type TN>
    class derived: public typed_base<TN>  {
        using this_type = derived<TN>;
    public:
        using sptr = std::shared_ptr<this_type>;

        static const type type_name = TN;
        using value_type = typename type2object<type_name>::native_type;

        derived(value_type val)
            :value_(val)
        { }

        std::string str( ) const override
        {
            std::ostringstream oss;
            oss << value( );
            return oss.str( );
        }

        value_type value( ) const
        {
            return value_;
        }

        void set_value( value_type val )
        {
            value_ = val;
        }

        template <typename T>
        static
        sptr make( T val )
        {
            return std::make_shared<this_type>( static_cast<value_type>(val) );
        }

        static
        std::size_t hash(value_type x )
        {
            std::hash<value_type> h;
            return h(x);
        }

        bool equal( const base *other ) const override
        {
            if( other->get_type( ) == this->get_type( ) ) {
                auto o = static_cast<const this_type *>( other );
                return o->value( ) == value( );
            }
            return false;
        }

        std::shared_ptr<base> clone( ) const override
        {
            return make( value_ );
        }

    private:

        value_type value_;
    };

    struct hash_helper {

        std::size_t operator ( )(const objects::sptr &h) const
        {
            return static_cast<std::size_t>(h->hash( ));
        }
    };

    struct equal_helper {

        bool operator ( )( const objects::sptr &l,
                           const objects::sptr &r ) const
        {
            return l->equal( r.get( ) );
        }
    };

    template <>
    class derived<type::ERROR>: public typed_base<type::ERROR> {

        using this_type = derived<type::ERROR>;
    public:
        using sptr = std::shared_ptr<this_type>;

        using value_type = std::string;

        derived( tokens::position where, value_type val )
            :where_(where)
            ,value_(std::move(val))
        { }

        std::string str( ) const override
        {
            std::ostringstream oss;
            oss << "error: [" << where_ << "] " << value( );
            return oss.str( );
        }

        const value_type &value( ) const
        {
            return value_;
        }

        value_type &value( )
        {
            return value_;
        }

        std::shared_ptr<base> clone( ) const override
        {
            auto res = make( where_, value_ );
            return res;
        }

    private:

        static
        void out_err( std::ostream & ) { }

        template <typename HeadT, typename ...Rest>
        static
        void out_err( std::ostream &o, const HeadT &h, Rest&&...rest )
        {
            o << h;
            out_err( o, std::forward<Rest>(rest)... );
        }
    public:

        template <typename ...Args>
        static
        objects::sptr make( const ast::node *n, Args&&...args )
        {
            std::ostringstream oss;
            out_err( oss, std::forward<Args>(args)...);
            return make( n->pos( ), oss.str( ) );
        }

        template <typename ...Args>
        static
        objects::sptr make( tokens::position where, Args&&...args )
        {
            std::ostringstream oss;
            out_err( oss, std::forward<Args>(args)...);
            return std::make_shared<this_type>( where, oss.str( ) );
        }

        template <typename ...Args>
        static
        objects::sptr make( Args&&...args )
        {
            std::ostringstream oss;
            out_err( oss, std::forward<Args>(args)... );
            tokens::position pos(0, 0);
            return make(pos, oss.str( ) );
        }

    private:
        tokens::position where_;
        value_type value_;
    };

    template <>
    class derived<type::TABLE>: public typed_base<type::TABLE> {

        using this_type = derived<type::TABLE>;
    public:
        using sptr = std::shared_ptr<this_type>;
        using cont = derived<type::REFERENCE>;
        using cont_sptr = std::shared_ptr<cont>;

        using value_type = std::unordered_map<objects::sptr, cont_sptr,
                                              hash_helper, equal_helper>;

        std::string str( ) const override
        {
            std::ostringstream oss;
            oss << "{ ";
            for( auto &v: value_ ) {
                oss << v.first->str( ) << ":";
                oss << v.second->str( )<< " ";
            }
            oss << "}";
            return oss.str( );
        }

        const value_type &value( ) const
        {
            return value_;
        }

        value_type &value( )
        {
            return value_;
        }

        bool insert( objects::sptr key, objects::sptr val )
        {
            value_[key->clone( )] = cont::make(val);
            return true;
        }

        std::uint64_t hash( ) const override
        {
            auto h = static_cast<std::uint64_t>(get_type( ));
            for( auto &o: value( ) ) {
                h = base::hash64( h + o.first->hash( ) +
                                     o.second->hash( ) );
            }
            return h;
        }

        objects::sptr at( objects::sptr id )
        {
            objects::sptr ptr = id;
            auto f = value_.find( ptr );
            if(f == value_.end( )) {
                return derived<type::NULL_OBJ>::make( );
            } else {
                return f->second;
            }
        }

        bool equal( const base *other ) const override
        {
            if( other->get_type( ) == get_type( ) ) {
                auto o = static_cast<const this_type *>( other );
                if( o->value( ).size( ) == value( ).size( ) ) {

                    auto b0 = value( ).begin( );
                    auto b1 = o->value( ).begin( );

                    for( ;b0 != value( ).end( ); ++b0, ++b1 ) {
                        auto f = b0->first->equal( b1->first.get( ) );
                        if( f ) {
                            auto val = b1->second->value( ).get( );
                            auto s = b0->second->equal( val );
                            if( !s ) {
                                return false;
                            }
                        }
                    }
                    return true;
                }
            }
            return false;
        }

        static
        sptr make( )
        {
            return std::make_shared<this_type>( );
        }

        std::shared_ptr<base> clone( ) const override
        {
            using ref = derived<type::REFERENCE>;
            auto res = make( );
            for( auto &v: value( ) ) {
                auto kc = v.first->clone( );
                auto vc = ref::make( v.second->clone( ) );
                res->value( ).insert( std::make_pair(kc, vc) );
            }
            res->value_ = value_;
            return res;
        }

    private:

        void env_reset( ) override
        {
            for( auto &v: value_ ) {
                v.second->value( ).reset( );
                v.second.reset( );
            }
        }
        value_type value_;
    };

    using null       = derived<type::NULL_OBJ>;
    using string     = derived<type::STRING>;
    using function   = derived<type::FUNCTION>;
    using builtin    = derived<type::BUILTIN>;
    using cont_call  = derived<type::CONT_CALL>;
    using retutn_obj = derived<type::RETURN>;
    using boolean    = derived<type::BOOLEAN>;
    using integer    = derived<type::INTEGER>;
    using floating   = derived<type::FLOAT>;
    using array      = derived<type::ARRAY>;
    using reference  = derived<type::REFERENCE>;
    using table      = derived<type::TABLE>;
    using error      = derived<type::ERROR>;

    inline
    std::ostream &operator << ( std::ostream &o, const objects::sptr &obj )
    {
        return o << obj->str( );
    }

    inline
    std::ostream &operator << ( std::ostream &o, type t )
    {
        return o << name::get(t);
    }

}}

#endif // OBJECTS_H
