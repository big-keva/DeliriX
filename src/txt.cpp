# include "../DOM-text.hpp"
# include <moonycode/codes.h>
# include <mtc/wcsstr.h>

namespace DeliriX
{

  class Text::Markup final: public IText
  {
    friend class Text;

  protected:
    auto  AddMarkupTag( const std::string_view&, const markup_attribute& ) -> mtc::api<IText> override;
    auto  AddParagraph( const char_string_view&, uint32_t ) -> Paragraph override;
    auto  AddParagraph( const wide_string_view& ) -> Paragraph override;
    auto  AddParagraph( const Paragraph& ) -> Paragraph override;

    void  Close();

  public:
    Markup( Text*, const std::string_view& );
    Markup( Markup*, const std::string_view& );
   ~Markup();

    implement_lifetime_control

  protected:
    mtc::api<Text>    docptr;
    Markup**          ppself;
    Markup*           nested;
    size_t            tagBeg;

  };

  // Text implementation

  Text::Text( int ): refCount( 0 ) {}

  Text::Text(): refCount( 1 ) {}

  Text::Text( const std::initializer_list<InitIt>& initlist, unsigned encoding ): refCount( 1 )
  {
    auto  fnFill = std::function<void( mtc::api<IText>, const std::initializer_list<InitIt>& )>();

    encoding = encoding == default_codepage ? codepages::codepage_utf8 : encoding;

    fnFill = [&]( mtc::api<IText> to, const std::initializer_list<InitIt>& it )
    {
      for ( auto& next: it )
        if ( next.psz != nullptr )  to->AddParagraph( { next.psz, next.cch }, encoding );
          else
        if ( next.wsz != nullptr )  to->AddParagraph( wide_string_view{ next.wsz, next.cch } );
          else
        fnFill( to->AddMarkupTag( next.tag ), *next.arr );
    };

    fnFill( this, initlist );
  }

  Text::~Text()
  {
    clear();
  }

  auto  Text::Create() -> mtc::api<Text>
  {
    auto  palloc = new std::aligned_storage_t<sizeof(Text), alignof(Text)>;

    return new( palloc ) Text( 1 );
  }

  long  Text::Attach() noexcept
  {
    return ++refCount;
  }

  long  Text::Detach()
  {
    auto  rcount = --refCount;

    if ( rcount == 0 )
    {
      this->~Text();
      delete (std::aligned_storage_t<sizeof(Text), alignof(Text)>*)this;
    }
    return rcount;
  }

  auto  Text::AddMarkupTag( const std::string_view& tag, const markup_attribute& ) -> mtc::api<IText>
  {
    if ( nested != nullptr )
      nested->Close();

    return nested = new Markup( this, tag );
  }

  auto  Text::AddParagraph( const char_string_view& str, uint32_t enc ) -> Paragraph
  {
    if ( nested != nullptr )
      nested->Close();

    blocks.emplace_back( Paragraph( str.data(), str.length(), enc == default_codepage ? codepages::codepage_utf8 : enc ) );
      length += blocks.back().GetTextSize();
    return blocks.back();
  }

  auto  Text::AddParagraph( const wide_string_view& wcs ) -> Paragraph
  {
    if ( nested != nullptr )
      nested->Close();

    blocks.emplace_back( Paragraph( wcs.data(), wcs.length() ) );
      length += blocks.back().GetTextSize();
    return blocks.back();
  }

  auto  Text::AddParagraph( const Paragraph& p ) -> Paragraph
  {
    if ( nested != nullptr )
      nested->Close();

    blocks.emplace_back( p );
      length += p.GetTextSize();
    return blocks.back();
  }

  void  Text::clear()
  {
    if ( nested != nullptr )
      nested->Close();
    blocks.clear();
    markup.clear();
    length = 0;
  }

  auto  Text::GetBufLen() const -> size_t
  {
    auto  length = ::GetBufLen( blocks.size() ) + ::GetBufLen( markup.size() );

    for ( auto& str: blocks )
    {
      auto  enc = str.GetEncoding();
      auto  len = str.GetTextSize();

      length +=
        ::GetBufLen( enc + 1 ) + ::GetBufLen( len ) + len * (enc == unsigned(-1) ? sizeof(widechar) : sizeof(char));
    }

    for ( auto& tag: markup )
    {
      length +=
        ::GetBufLen( tag.tagKey )
      + ::GetBufLen( tag.uLower )
      + ::GetBufLen( tag.uUpper );
    }
    return length;
  }

  auto  Text::Serialize( IText* text ) const -> IText*
  {
    return DeliriX::Serialize( text, *this );
  }

  // Text::Markup implementation
  
  Text::Markup::Markup( Text* owner, const std::string_view& tag ):
    docptr( owner ),
    ppself( &owner->nested ),
    nested( nullptr ),
    tagBeg( owner->markup.size() )
  {
    owner->markup.push_back( { std::string( tag.data(), tag.length() ), uint32_t(owner->length), uint32_t(-1) } );
  }

  Text::Markup::Markup( Markup* owner, const std::string_view& tag ):
    docptr( owner->docptr ),
    ppself( &owner->nested ),
    nested( nullptr ),
    tagBeg( docptr->markup.size() )
  {
    docptr->markup.push_back( { std::string( tag.data(), tag.length() ), uint32_t(docptr->length), uint32_t(-1) } );
  }

  Text::Markup::~Markup()
  {
    Close();
  }

  auto  Text::Markup::AddMarkupTag( const std::string_view& tag, const markup_attribute& ) -> mtc::api<IText>
  {
    if ( nested != nullptr )
      nested->Close();

    return nested = new Markup( this, tag );
  }

  auto  Text::Markup::AddParagraph( const char_string_view& str, unsigned enc ) -> Paragraph
  {
    if ( tagBeg == size_t(-1) )
      throw std::logic_error( "attempt of adding line to closed markup" );

    if ( nested != nullptr )
      nested->Close();

    docptr->blocks.emplace_back( str.data(), str.length(), enc == default_codepage ? codepages::codepage_utf8 : enc );
      docptr->length += docptr->blocks.back().GetTextSize();

    return docptr->blocks.back();
  }

  auto  Text::Markup::AddParagraph( const wide_string_view& str ) -> Paragraph
  {
    if ( tagBeg == size_t(-1) )
      throw std::logic_error( "attempt of adding line to closed markup" );

    if ( nested != nullptr )
      nested->Close();

    docptr->blocks.emplace_back( str.data(), str.length() );
      docptr->length += docptr->blocks.back().GetTextSize();
    return docptr->blocks.back();
  }

  auto  Text::Markup::AddParagraph( const Paragraph& str ) -> Paragraph
  {
    if ( tagBeg == size_t(-1) )
      throw std::logic_error( "attempt of adding line to closed markup" );

    if ( nested != nullptr )
      nested->Close();

    docptr->blocks.emplace_back( str );
      docptr->length += str.GetTextSize();
    return docptr->blocks.back();
  }

  void  Text::Markup::Close()
  {
    if ( tagBeg != size_t(-1) )
    {
      if ( nested != nullptr )
        nested->Close();

      if ( *ppself != nullptr )
        *ppself = nullptr;

      if ( docptr->length < docptr->markup[tagBeg].uLower + 1 )
      {
        assert( docptr->markup.size() == tagBeg + 1 );

        docptr->markup.pop_back();
      } else docptr->markup[tagBeg].uUpper = docptr->length - 1;

      tagBeg = size_t(-1);
    }
  }

}
