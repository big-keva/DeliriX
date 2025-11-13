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
    auto  AddMarkupTag( const std::string_view& tag, const markup_attribute& ) -> mtc::api<IText> override
    {
      if ( nItems++ != 0 )  fWrite( ",\n", 2 );
        else fWrite( "\n", 1 );

      return new JsonTag( fWrite, uShift + 1, { tag.data(), tag.length() } );
    }
    auto  AddParagraph( const Paragraph& str ) -> Paragraph override
    {
      auto  utfstr = std::string();
      auto  asView = str.GetCharStr();
      auto  coding = str.GetEncoding();

      switch ( coding )
      {
        case uint32_t(-1):
          asView = (utfstr = codepages::widetombcs( codepages::codepage_utf8, str.GetWideStr() ));
        case codepages::codepage_utf8:
          break;
        default:
          asView = (utfstr = codepages::mbcstombcs( codepages::codepage_utf8, coding, asView ));
          break;
      }

      if ( nItems != 0 )  fWrite( ",\n", 2 );
        else fWrite( "\n", 1 );

      for ( unsigned u = 0; u != uShift + 1; ++u )
        fWrite( "  ", 2 );

      fWrite( "\"", 1 );
        Print( asView );
      fWrite( "\"", 1 );

      ++nItems;

      return {};
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
