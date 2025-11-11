# include "../DOM-dump.hpp"
# include <moonycode/codes.h>

// tags implementation

namespace DeliriX {
namespace dump_as {

  using FnSink = std::function<void(const char*, size_t)>;

  class JsonTag final: public IText
  {
    FnSink      fWrite;
    unsigned    uShift;
    size_t      nItems = 0;
    bool        is_Tag;

    implement_lifetime_control

  public:
    JsonTag( FnSink f, unsigned u, const std::string& t = {} ):
      fWrite( f ),
      uShift( u ),
      is_Tag( !t.empty() )
    {
      if ( is_Tag )
      {
        for ( unsigned u = 0; u != uShift; ++u )
          fWrite( "  ", 2 );

        fWrite( "{ \"", 3 );
          Print( t );
        fWrite( "\": ", 3 );
      }
      fWrite( "[\n", 1 );
    }
    ~JsonTag()
    {
      if ( nItems != 0 )
        fWrite( "\n", 1 );

      for ( unsigned u = 0; u != uShift; ++u )
        fWrite( "  ", 2 );

      fWrite( "]", 1 );

      if ( is_Tag )
        fWrite( " }", 2 );
    }
    auto  AddMarkupTag( const char_string_view& tag, const markup_attribute& ) -> mtc::api<IText> override
    {
      if ( nItems++ != 0 )  fWrite( ",\n", 2 );
        else fWrite( "\n", 1 );

      return new JsonTag( fWrite, uShift + 1, { tag.data(), tag.length() } );
    }
    auto  AddParagraph( const char_string_view& str, uint32_t encode ) -> Paragraph override
    {
      if ( nItems != 0 )  fWrite( ",\n", 2 );
        else fWrite( "\n", 1 );

      for ( unsigned u = 0; u != uShift + 1; ++u )
        fWrite( "  ", 2 );

      fWrite( "\"", 1 );

      if ( (encode = encode == default_codepage ? codepages::codepage_utf8 : encode) != codepages::codepage_utf8 )
        Print( codepages::mbcstombcs( codepages::codepage_utf8, encode, str ) );
      else Print( str );

      fWrite( "\"", 1 );

      ++nItems;

      return {};
    }
    auto  AddParagraph( const wide_string_view& str ) -> Paragraph override
    {
      return AddParagraph( codepages::widetombcs( codepages::codepage_utf8, str ),
        codepages::codepage_utf8 );
    }
    auto  AddParagraph( const Paragraph& str ) -> Paragraph override
    {
      return str.GetEncoding() == uint32_t(-1) ? AddParagraph( str.GetWideStr() )
        : AddParagraph( str.GetCharStr(), str.GetEncoding() );
    }
  protected:
    void  Print( const std::string_view& src ) const
    {
      static const char escapeChar[] = "\"\\/\b\f\n\r\t";

      for ( auto ptr = src.data(); ptr != src.end(); )
      {
        auto  org = ptr;

        while ( ptr != src.end() && strchr( escapeChar, *ptr ) == nullptr )
          ++ptr;

        if ( ptr != org ) fWrite( org, ptr - org );
          else
        switch ( *ptr++ )
        {
          case '\"':  fWrite( "\\\"", 2 );  break;
          case '\\':  fWrite( "\\\\", 2 );  break;
          case '/':   fWrite( "\\/",  2 );  break;
          case '\b':  fWrite( "\\b", 2 );  break;
          case '\f':  fWrite( "\\f", 2 );  break;
          case '\n':  fWrite( "\\n", 2 );  break;
          case '\r':  fWrite( "\\r", 2 );  break;
          case '\t':  fWrite( "\\t", 2 );  break;
          default: throw std::logic_error( "invalid escape sequence" );
        }
      }
    }
  };

  auto  Json( std::function<void( const char*, size_t )> fn ) -> mtc::api<IText>
  {
    return new JsonTag( fn, 0 );
  }

}}
