# if !defined( __DeliriX_textAPI_hpp__ )
# define __DeliriX_textAPI_hpp__
# include <mtc/interfaces.h>
# include <mtc/span.hpp>
# include <functional>
# include <stdexcept>
# include <map>

namespace DeliriX
{

  const unsigned default_codepage = (unsigned)-1;

  struct MarkupTag
  {
    std::string tagKey;
    uint32_t    uLower;
    uint32_t    uUpper;
  };

  class Paragraph
  {
    union
    {
      const char*     charstr;
      const widechar* widestr;
    };

  public:
    Paragraph();
    Paragraph( const char* str, uint32_t len, uint32_t enc );
    Paragraph( const widechar* str, uint32_t len );
   ~Paragraph();
    Paragraph( const Paragraph& );
    Paragraph( Paragraph&& );
    Paragraph& operator = ( const Paragraph& );
    Paragraph& operator = ( Paragraph&& );

    uint32_t  GetEncoding() const;
    uint32_t  GetTextSize() const;

    auto      GetCharStr() const -> std::string_view;
    auto      GetWideStr() const -> std::basic_string_view<widechar>;

    bool      Serialize( std::function<bool( const void*, size_t )> ) const;
    bool      FetchFrom( std::function<bool( void*, size_t )> );
  };

  struct IText: mtc::Iface
  {
    using char_string_view = std::basic_string_view<char>;
    using wide_string_view = std::basic_string_view<widechar>;
    using markup_attribute = std::map<std::string, std::string>;

    virtual auto  AddMarkupTag( const char_string_view&, const markup_attribute& = {} ) -> mtc::api<IText>  = 0;
    virtual auto  AddParagraph( const char_string_view&, uint32_t = default_codepage ) -> Paragraph = 0;
    virtual auto  AddParagraph( const wide_string_view& ) -> Paragraph = 0;
    virtual auto  AddParagraph( const Paragraph& ) -> Paragraph = 0;

  };

  struct ITextView: mtc::Iface
  {
    virtual auto  GetBlocks() const -> mtc::span<const Paragraph> = 0;
    virtual auto  GetMarkup() const -> mtc::span<const MarkupTag> = 0;
    virtual auto  GetLength() const -> uint32_t = 0;
  };

}

# endif   // !__DeliriX_textAPI_hpp__
