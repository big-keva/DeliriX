# include "xml.hpp"
# include <moonycode/codes.h>
# include <mtc/wcsstr.h>
# include <tinyxml2.h>
# include <map>

namespace DeliriX::tinyxml
{

  class Parser
  {
    unsigned  encode = codepages::codepage_utf8;

  public:
    auto  Decl( IText* text, const char* decl ) -> IText*;
    auto  Elem( IText* text, const tinyxml2::XMLElement& ) -> IText*;
    auto  List( IText* text, const tinyxml2::XMLElement& ) -> IText*;
    auto  Load( IText* text, const tinyxml2::XMLNode& ) -> IText*;

  protected:
    auto  Decl( const char* ) const -> std::map<std::string, std::string>;

  };

  auto  Parser::Decl( IText* text, const char* dcl ) -> IText*
  {
    auto  decmap = Decl( dcl );
    auto  encode = decmap.find( "encoding" );

    if ( encode != decmap.end() )
    {
      if ( mtc::w_strcasecmp( encode->second.c_str(), "utf-8" ) == 0 )
      {
        this->encode = codepages::codepage_utf8;
      }
        else
      if ( mtc::w_strcasecmp( encode->second.c_str(), "windows-1251" ) == 0
        || mtc::w_strcasecmp( encode->second.c_str(), "windows-1252" ) == 0 )
      {
        this->encode = codepages::codepage_1251;
      }
        else
      if ( mtc::w_strcasecmp( encode->second.c_str(), "iso-8859" ) == 0 )
      {
        this->encode = codepages::codepage_iso;
      }
        else
      throw Error( mtc::strprintf( "invalid encoding '%s'", encode->second.c_str() ) );
    }
    return text;
  }

  auto  Parser::Elem( IText* text, const tinyxml2::XMLElement& elem ) -> IText*
  {
    if ( elem.Value() != nullptr )
    {
      auto  attrib = IText::markup_attribute();
      auto  addtag = mtc::api<IText>();

    // get provided attributes for tag
      for ( auto attr = elem.FirstAttribute(); attr != nullptr; attr = attr->Next() )
        attrib.emplace( attr->Name(), attr->Value() );

    // try add tag and add elements if the tag is added
      if ( (addtag = text->AddMarkupTag( elem.Value(), attrib )) != nullptr )
        for ( auto next = elem.FirstChild(); next != nullptr; next = next->NextSibling() )
          Load( addtag, *next );
    }
    return text;
  }

  auto  Parser::List( IText* text, const tinyxml2::XMLElement& elem ) -> IText*
  {
    if ( elem.Value() != nullptr )
    {
      for ( auto next = elem.FirstChild(); next != nullptr; next = next->NextSibling() )
        Load( text, *next );
    }
    return text;
  }

  auto  Parser::Load( IText* text, const tinyxml2::XMLNode& node ) -> IText*
  {
    if ( node.ToDeclaration() != nullptr )
      return Decl( text, node.ToDeclaration()->Value() );

    if ( node.ToElement() != nullptr )
      return Elem( text, *node.ToElement() );

    if ( node.ToText() != nullptr && node.ToText()->Value() != nullptr )
      text->AddBlock( encode, node.ToText()->Value() );

    return text;
  }

  auto  Parser::Decl( const char* decl ) const -> std::map<std::string, std::string>
  {
    auto  outmap = std::map<std::string, std::string>();

    for ( decl = mtc::ltrim( decl ); *decl != '\0'; decl = mtc::ltrim( decl ) )
    {
      auto  keytop = decl;
      auto  keyend = decl;

      while ( *keyend != '\0' && *keyend != '=' && !mtc::isspace( *keyend ) )
        ++keyend;

      if ( *keyend == '=' )
      {
        auto  valtop = mtc::ltrim( keyend + 1 );
        auto  valend = valtop;

        while ( *valend != '\0' && !mtc::isspace( *valend ) )
          ++valend;

        decl = valend;

        valtop = mtc::ltrim( valtop, "\'\"" );
        valend = mtc::rtrim( valtop, valend, "\'\"" );

        outmap.insert( { std::string( keytop, keyend ), std::string( valtop, valend ) } );
      }
        else
      outmap.insert( { std::string( keytop, decl = keyend ), "" } );
    }
    return outmap;
  }

  void  ParseXML( IText* text, const mtc::api<const mtc::IByteBuffer>& buff )
  {
    Parser                load;
    tinyxml2::XMLDocument xdoc;

    if ( buff == nullptr )
      throw std::invalid_argument( "XML source is null" );

    if ( xdoc.Parse( buff->GetPtr(), buff->GetLen() ) != tinyxml2::XML_SUCCESS )
      throw Error( mtc::strprintf( "failed to parse XML, error '%s:%s'",
        xdoc.ErrorName(), xdoc.ErrorStr() ) );

    for ( auto child = xdoc.FirstChild(); child != nullptr; child = child->NextSibling() )
      load.Load( text, *child );
  }

}
