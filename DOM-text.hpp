# if !defined( __DeliriX_DOM_text_hpp__ )
# define __DeliriX_DOM_text_hpp__
# include "text-API.hpp"
# include <mtc/serialize.h>

namespace DeliriX
{

  class Text final: public IText, public ITextView
  {
    class Markup;

    std::atomic_long  refCount;

    void* operator new( size_t ) = delete;
    void* operator new( size_t, void* p ) {  return p;  }
    void  operator delete( void* ) = delete;

    struct InitIt
    {
      friend class Text;

      InitIt( const char* s, size_t l = size_t(-1) ):
        psz( s ),
        wsz( nullptr ),
        cch( l ),
        tag( nullptr ),
        arr( nullptr )  {}
      InitIt( const widechar* s, size_t l = size_t(-1) ):
        psz( nullptr ),
        wsz( s ),
        cch( l ),
        tag( nullptr ),
        arr( nullptr )  {}
      InitIt( const char* t, const std::initializer_list<InitIt>& l ):
        psz( nullptr ),
        wsz( nullptr ),
        tag( t ),
        arr( &l ) {}
      template <class StringAllocator>
        InitIt( const std::basic_string<char, std::char_traits<char>, StringAllocator>& s ):
          InitIt( s.c_str(), s.length() ) {}
      template <class StringAllocator>
        InitIt( const std::basic_string<widechar, std::char_traits<widechar>, StringAllocator>& s ):
          InitIt( s.c_str(), s.length() ) {}

    protected:
      const char*                           psz;
      const widechar*                       wsz;
      size_t                                cch = size_t(-1);
      const char*                           tag;
      const std::initializer_list<InitIt>*  arr;
    };

  protected:
    Text( int );

  public:
    Text();
    Text( const std::initializer_list<InitIt>&, unsigned cp = default_codepage );
   ~Text();

  // head creation helper
    static  auto  Create() -> mtc::api<Text>;

  // lifetime control overridables
    long  Attach() noexcept override;
    long  Detach() override;

  // IText overridables
    auto  AddMarkupTag( const char_string_view&, const markup_attribute& = {} ) -> mtc::api<IText>  override;
    auto  AddParagraph( const char_string_view&, uint32_t = default_codepage ) -> Paragraph override;
    auto  AddParagraph( const wide_string_view& ) -> Paragraph override;
    auto  AddParagraph( const Paragraph& )        -> Paragraph override;

  // ITextView overridables
    auto  GetBlocks() const -> mtc::span<const Paragraph> override  {  return blocks;  }
    auto  GetMarkup() const -> mtc::span<const MarkupTag> override  {  return markup;  }
    auto  GetLength() const -> uint32_t override                    {  return length;  };

  // modification
    void  clear();

  // serialization
    auto  GetBufLen() const -> size_t;
  template <class O>
    auto  Serialize( O* ) const -> O*;
  template <class S>
    auto  FetchFrom( S* ) -> S*;
    auto  Serialize( IText* ) const -> IText*;

  protected:
    std::vector<Paragraph>  blocks;
    std::vector<MarkupTag>  markup;
    Markup*                 nested = nullptr;
    uint32_t                length = 0;

  };

  bool  IsEncoded( const ITextView&, unsigned encoding );
  auto  CopyUtf16( IText*, const ITextView&, unsigned default_encoding = default_codepage ) -> IText*;
  auto  Serialize( IText*, const ITextView& ) -> IText*;

// Text template implementation

  template <class O>
  O*  Text::Serialize( O* o ) const
  {
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

    return o;
  }

  template <class S>
  S*  Text::FetchFrom( S* s )
  {
    size_t  length;

    clear();

  // get strings vector length value
    if ( (s = ::FetchFrom( s, length )) != nullptr )  blocks.resize( length );
      else return nullptr;

  // get strings
    for ( auto& str: blocks )
    {
      if ( !str.FetchFrom( [&]( void* p, size_t l ){  return (s = ::FetchFrom( s, p, l )) != nullptr;  } ) )
        return s;
      length += str.GetTextSize();
    }

    if ( (s = ::FetchFrom( s, length )) != nullptr )  markup.resize( length );
      else return nullptr;

    for ( auto& tag: markup )
    {
      size_t  taglen;

      if ( (s = ::FetchFrom( s, taglen )) == nullptr )
        return nullptr;

      tag.tagKey.resize( taglen );

      s = ::FetchFrom( ::FetchFrom( ::FetchFrom( s, (void*)tag.tagKey.c_str(), taglen ),
        tag.uLower ),
        tag.uUpper );

      if ( s == nullptr )
        return nullptr;
    }

    return s;
  }

}

# endif   // !__DeliriX_DOM_text_hpp__
