# include "../DOM-dump.hpp"
# include <moonycode/codes.h>

namespace DeliriX {
namespace dump_as {

  using FnSink = std::function<void(const char*, size_t)>;

  class TagsTag final: public IText
  {
    FnSink          fWrite;
    const unsigned  encode = codepages::codepage_utf8;
    unsigned        uShift;
    std::string     tagStr;

    implement_lifetime_control

  public:
    TagsTag( FnSink f, unsigned c, unsigned u, std::string&& t = {} ):
      fWrite( f ),
      encode( c ),
      uShift( u ),
      tagStr( std::move( t ) )
    {
      if ( !tagStr.empty() )
      {
        for ( unsigned u = 0; u != uShift; ++u )
          fWrite( "  ", 2 );

        fWrite( "<", 1 );
          Print( tagStr );
        fWrite( ">\n", 2 );
      }
    }
    ~TagsTag()
    {
      if ( !tagStr.empty() )
      {
        for ( unsigned u = 0; u != uShift; ++u )
          fWrite( "  ", 2 );

        fWrite( "</", 2 );
          Print( tagStr );
        fWrite( ">\n", 2 );
      }
    }
    auto  AddMarkupTag( const std::string_view& tag, const markup_attribute& ) -> mtc::api<IText> override
    {
      return new TagsTag( fWrite, encode, uShift + 1, { tag.data(), tag.length() } );
    }
    auto  AddParagraph( const Paragraph& src ) -> Paragraph override
    {
      std::string       utfstr;
      std::string_view  asView;
      auto              coding = src.GetEncoding();

      switch ( coding )
      {
        case uint32_t(-1):
          asView = (utfstr = codepages::widetombcs( codepages::codepage_utf8, src.GetWideStr() ));
          break;
        case codepages::codepage_utf8:
          asView = src.GetCharStr();
          break;
        default:
          asView = (utfstr = codepages::mbcstombcs( codepages::codepage_utf8, coding, src.GetCharStr() ));
          break;
      }

      if ( !tagStr.empty() )
      {
        for ( unsigned u = 0; u <= uShift; ++u )
          fWrite( "  ", 2 );
      }

      Print( asView );

      fWrite( "\n", 1 );

      return {};
    }
  protected:
    void  Print( const std::string_view& src ) const
    {
      for ( auto ptr = src.begin(); ptr != src.end(); )
      {
        auto org = ptr;

        while ( ptr != src.end() && *ptr != '<' && *ptr != '>' && *ptr != '&' )
          ++ptr;

        if ( ptr != org ) fWrite( org, ptr - org );
          else
        switch ( *ptr++ )
        {
          case '<': fWrite( "&lt;", 4 );  break;
          case '>': fWrite( "&gt;", 4 );  break;
          default:  fWrite( "&amp;", 5 );
        }
      }
    }
  };

  auto  Tags( std::function<void( const char*, size_t )> fn, unsigned cp ) -> mtc::api<IText>
  {
    return new TagsTag( fn, cp, unsigned(-1) );
  }

}}
