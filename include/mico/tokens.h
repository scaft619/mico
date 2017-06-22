#ifndef MICO_TOKENS_H
#define MICO_TOKENS_H

#include <cstdint>
#include <utility>
#include <string>
#include <ostream>

namespace mico { namespace tokens {

    enum class type {

        NONE        = 0,
        END_OF_LINE,
        END_OF_FILE,

        INT_BIN,
        INT_TER,
        INT_OCT,
        INT_DEC,
        INT_HEX,
        FLOAT,
        IDENT,
        STRING,

        FIRST_VISIBLE = 100,

        /// symbols

        SEMICOLON,
        COLON,
        DOT,
        COMMA,

        /// operators
        ASSIGN,
        MINUS,
        PLUS,
        BANG,
        ASTERISK,
        SLASH,
        EQ,
        NOT_EQ,
        LT,
        GT,

        LPAREN,
        RPAREN,
        LBRACE,
        RBRACE,
        LBRACKET,
        RBRACKET,

        /// keywords
        LET,
        RETURN,
        FUNCTION,
        BOOL_TRUE,
        BOOL_FALSE,
        IF,
        ELIF,
        ELSE,

        LAST_VISIBLE,
    };

    struct name {

        static
        const char *get( type t )
        {
            switch( t ) {
            case type::END_OF_FILE: return "EOF";
            case type::END_OF_LINE: return "EOL";
            case type::IDENT:       return "IDENT";

            case type::INT_BIN:     return "INT_2";
            case type::INT_TER:     return "INT_3";
            case type::INT_OCT:     return "INT_8";
            case type::INT_DEC:     return "INT_10";
            case type::INT_HEX:     return "INT_16";
            case type::FLOAT:       return "FLOAT";
            case type::STRING:      return "STRING";

            /// tokens that have names
            case type::SEMICOLON:   return ";";
            case type::COLON:       return ":";
            case type::DOT:         return ".";
            case type::COMMA:       return ",";

            /// operators
            case type::ASSIGN:      return "=";
            case type::MINUS:       return "-";
            case type::PLUS:        return "+";
            case type::BANG:        return "!";
            case type::ASTERISK:    return "*";
            case type::SLASH:       return "/";
            case type::EQ:          return "==";
            case type::NOT_EQ:      return "!=";
            case type::LT:          return "<";
            case type::GT:          return ">";

            case type::LPAREN:      return "(";
            case type::RPAREN:      return ")";
            case type::LBRACE:      return "{";
            case type::RBRACE:      return "}";
            case type::LBRACKET:    return "[";
            case type::RBRACKET:    return "]";

            /// keywords
            case type::LET:         return "let";
            case type::RETURN:      return "return";
            case type::FUNCTION:    return "fn";
            case type::BOOL_TRUE:   return "true";
            case type::BOOL_FALSE:  return "false";
            case type::IF:          return "if";
            case type::ELIF:        return "elif";
            case type::ELSE:        return "else";

            /// unnamed
            case type::NONE:
            case type::FIRST_VISIBLE:
            case type::LAST_VISIBLE:
                break;
            }
            return "";
        }
    };

    struct type_ident {

        using value_type = std::string;

        type_ident( ) = default;
        type_ident( const type_ident & ) = default;
        type_ident& operator = ( const type_ident & ) = default;

        type_ident( type tt )
            :name(tt)
            ,literal(name::get(tt))
        { }

        type_ident( type tt, std::string val )
            :name(tt)
            ,literal(std::move(val))
        { }

        type_ident( type_ident &&other )
            :name(other.name)
            ,literal(std::move(other.literal))
        { }

        type_ident& operator = ( type_ident &&other )
        {
            name     = other.name;
            literal = std::move(other.literal);
            return *this;
        }

        type        name;
        value_type  literal;
    };

    struct position {
        std::size_t line = 0;
        std::size_t pos  = 0;
        position(std::size_t l, std::size_t p )
            :line(l)
            ,pos(p)
        { }
        position( const position & ) = default;
        position(  ) = default;
    };

    struct info {

        using value_type = std::string;

        info( ) = default;
        info( const info & ) = default;
        info& operator = ( const info & ) = default;


        info( type t )
            :ident(t)
        { }

        info( type t, std::string value )
            :ident(t, std::move(value))
        { }

        info( info &&other )
            :where(other.where)
            ,ident(std::move(other.ident))
        { }

        info& operator = ( info &&other )
        {
            where = other.where;
            ident = std::move(other.ident);
            return *this;
        }

        position where;
        type_ident ident;
    };

    inline
    std::ostream &operator << (std::ostream &o, tokens::type tt )
    {
        return o << name::get(tt);
    }

    inline
    std::ostream &operator << (std::ostream &o, const tokens::position &tt )
    {
        return o << tt.line << ":" << tt.pos;
    }

    inline
    std::ostream &operator << (std::ostream &o, const tokens::type_ident &ti )
    {
        o << ti.name;
        if( ti.name < tokens::type::FIRST_VISIBLE ||
                      tokens::type::LAST_VISIBLE < ti.name )
        {
            o << "(" << ti.literal << ")";
        }
        return o;
    }

}}

#endif // TOKENS_H
