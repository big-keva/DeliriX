# if !defined( __DeliriX_textAPI_hpp__ )
# define __DeliriX_textAPI_hpp__
# include <mtc/interfaces.h>
# include <mtc/serialize.h>
# include <mtc/span.hpp>
# include <functional>
# include <map>

namespace DeliriX
{

  struct MarkupTag
  {
    std::string tagKey;
    uint32_t    uLower;
    uint32_t    uUpper;

    bool  operator == ( const MarkupTag& r ) const
      {  return tagKey == r.tagKey && uLower == r.uLower && uUpper == r.uUpper;  }
    bool  operator != ( const MarkupTag& r ) const
      {  return !(*this == r);  }
  };

  class Paragraph
  {
    friend class IText;

    union
    {
      const char*     charstr;
      const widechar* widestr;
    };

  public:
    Paragraph();
    Paragraph( Paragraph&& );
    Paragraph( const Paragraph& );
   ~Paragraph();
    Paragraph& operator = ( const Paragraph& );
    Paragraph& operator = ( Paragraph&& );

    uint32_t  GetEncoding() const;
    uint32_t  GetTextSize() const;

    auto      GetCharStr() const -> std::string_view;
    auto      GetWideStr() const -> std::basic_string_view<widechar>;

    size_t    GetBufLen() const;
    bool      Serialize( std::function<bool( const void*, size_t )> ) const;
    bool      FetchFrom( std::function<bool( void*, size_t )> );
  };

  struct IText: mtc::Iface
  {
    using char_string_view = std::basic_string_view<char>;
    using wide_string_view = std::basic_string_view<widechar>;

    using markup_attribute = std::map<std::string, std::string>;

    virtual auto  AddMarkupTag( const std::string_view&, const markup_attribute& = {} ) -> mtc::api<IText>  = 0;
    virtual auto  AddParagraph( const Paragraph& ) -> Paragraph = 0;

    auto  AddBlock( const char_string_view& str ) -> Paragraph
      {  return AddBlock( 0, str.data(), str.length() );  }
    auto  AddBlock( uint32_t cp, const char_string_view& str ) -> Paragraph
      {  return AddBlock( cp, str.data(), str.length() );  }
    auto  AddBlock( const char* str, uint32_t len ) -> Paragraph
      {  return AddBlock( 0, str, len );  }
    auto  AddBlock( uint32_t cp, const char*, uint32_t ) -> Paragraph;
    auto  AddBlock( const wide_string_view& str ) -> Paragraph
      {  return AddBlock( str.data(), str.size() );  }
    auto  AddBlock( const widechar* str, uint32_t len ) -> Paragraph;
  };

  struct ITextView: mtc::Iface
  {
    virtual auto  GetBlocks() const -> mtc::span<const Paragraph> = 0;
    virtual auto  GetMarkup() const -> mtc::span<const MarkupTag> = 0;
    virtual auto  GetLength() const -> uint32_t = 0;

    auto    GetBufLen() const -> size_t;
    template <class O>
    O*      Serialize( O* ) const;
    IText*  Serialize( IText* ) const;
  };

  bool  IsEncoded( const ITextView&, uint32_t encoding );
  auto  CopyUtf16( IText*, const ITextView&, uint32_t default_encoding = 0 ) -> IText*;

  template <class O>
  O*    ITextView::Serialize( O* o ) const
  {
    auto  blocks = GetBlocks();
    auto  markup = GetMarkup();
    auto  length = GetLength();

    o = ::Serialize( o, blocks.size() );

    for ( auto& str: blocks )
      if ( !str.Serialize( [&]( const void* p, size_t l ){  return (o = ::Serialize( o, p, l )) != nullptr;  } ) )
        return o;

    o = ::Serialize( o, markup.size() );

    for ( auto& tag: markup )
    {
      o = ::Serialize( ::Serialize( ::Serialize( o,
        tag.tagKey ),
        tag.uLower ),
        tag.uUpper );
    }

    return ::Serialize( o, length );
  }

}

# endif   // !__DeliriX_textAPI_hpp__
