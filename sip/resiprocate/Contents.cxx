#include "sip2/sipstack/Contents.hxx"
#include "sip2/util/ParseBuffer.hxx"
#include "sip2/util/Logger.hxx"

using namespace Vocal2;

#define VOCAL_SUBSYSTEM Subsystem::SIP

MIME_Transfer_Encoding_Header Vocal2::h_Transfer_Encoding;
MIME_Id_Header Vocal2::h_Id;
MIME_Description_Header Vocal2::h_Description;

std::map<Mime, ContentsFactoryBase*>* Contents::FactoryMap = 0;

Contents::Contents(HeaderFieldValue* headerFieldValue,
                   const Mime& contentType) 
   : LazyParser(headerFieldValue),
     mType(contentType),
     mDisposition(0),
     mTransferEncoding(0),
     mLanguages(0),
     mId(0),
     mDescription(0),
     mVersion(1),
     mMinorVersion(0),
     mHeadersFromMessage(false)
{}

Contents::Contents(const Mime& contentType) 
   : mType(contentType),
     mDisposition(0),
     mTransferEncoding(0),
     mLanguages(0),
     mId(0),
     mDescription(0),
     mVersion(1),
     mMinorVersion(0),
     mHeadersFromMessage(false)
{}

Contents::Contents(const Contents& rhs) 
    : LazyParser(rhs),
      mType(rhs.mType),
      mDisposition(0),
      mTransferEncoding(0),
      mLanguages(0),
      mId(0),
      mDescription(0),
      mVersion(1),
      mMinorVersion(0),
      mHeadersFromMessage(rhs.mHeadersFromMessage)
{
   *this = rhs;
}

Contents::~Contents()
{
}

void
Contents::clear()
{
   delete mDisposition;
   delete mTransferEncoding;
   delete mLanguages;
   delete mId;
   delete mDescription;

   mDisposition = 0;
   mTransferEncoding = 0;
   mLanguages = 0;
   mId = 0;
   mDescription = 0;
}

Contents& 
Contents::operator=(const Contents& rhs) 
{
   if (this != &rhs)
   {
      LazyParser::operator=(rhs); 
      mType = rhs.mType;
      if (rhs.mDisposition)
      {
         mDisposition = new Content_Disposition_Header::Type(*rhs.mDisposition);
      }
      if (rhs.mTransferEncoding)
      {
         mTransferEncoding = new Token(*rhs.mTransferEncoding);
      }
      if (rhs.mLanguages)
      {
         mLanguages = new ParserContainer<Content_Language_MultiHeader::Type>(*rhs.mLanguages);
      }
      if (rhs.mId)
      {
         mId = new Token(*rhs.mId);
      }
      if (rhs.mDescription)
      {
         mDescription = new StringCategory(*rhs.mDescription);
      }

      mVersion = rhs.mVersion;
      mMinorVersion = rhs.mMinorVersion;
      mHeadersFromMessage = rhs.mHeadersFromMessage;
   }

   return *this;
}

std::map<Mime, ContentsFactoryBase*>& 
Contents::getFactoryMap()
{
   if (Contents::FactoryMap == 0)
   {
      Contents::FactoryMap = new std::map<Mime, ContentsFactoryBase*>();
   }
   return *Contents::FactoryMap;
}

Contents*
Contents::getContents(const Mime& m)
{
   assert(Contents::getFactoryMap().find(m) != Contents::getFactoryMap().end());
   return Contents::getFactoryMap()[m]->convert(getContents());
}

Contents*
Contents::createContents(const Mime& contentType, 
                         const Data& contents)
{
   assert(!contents.mMine);
   HeaderFieldValue *hfv = new HeaderFieldValue(contents.data(), contents.size());
   assert(Contents::getFactoryMap().find(contentType) != Contents::getFactoryMap().end());
   Contents* c = Contents::getFactoryMap()[contentType]->create(hfv, contentType);
   c->mIsMine = true;
   return c;
}

bool
Contents::exists(const HeaderBase& headerType) const
{
   checkParsed();
   switch (headerType.getTypeNum())
   {
      case Headers::Content_Type :
      {
         return true;
      }
      case Headers::Content_Disposition :
      {
         return mDisposition != 0;
      }
      case Headers::Content_Language :
      {
         return mLanguages != 0;
      }
      default : return false;
   }
}

bool
Contents::exists(const MIME_Header& type) const
{
   if (&type == &h_Transfer_Encoding)
   {
      return mTransferEncoding != 0;
   }

   if (&type == &h_Id)
   {
      return mId != 0;
   }
   
   if (&type == &h_Description)
   {
      return mDescription != 0;
   }

   assert(false);
   return false;
}

void
Contents::remove(const HeaderBase& headerType)
{
   switch (headerType.getTypeNum())
   {
      case Headers::Content_Disposition :
      {
         delete mDisposition;
         mDisposition = 0;
         break;
      }
      case Headers::Content_Language :
      {
         delete mLanguages;
         mLanguages = 0;
         break;
      }
      default :
         ;
   }
}

void
Contents::remove(const MIME_Header& type)
{
   if (&type == &h_Transfer_Encoding)
   {
      delete mTransferEncoding;
      mTransferEncoding = 0;
      return;
   }

   if (&type == &h_Id)
   {
      delete mId;
      mId = 0;
      return;
   }
    
   if (&type == &h_Description)
   {
      delete mDescription;
      mDescription = 0;
      return;
   }

   assert(false);
}

Content_Type_Header::Type&
Contents::header(const Content_Type_Header& headerType) const
{
   return mType;
}

Content_Disposition_Header::Type&
Contents::header(const Content_Disposition_Header& headerType) const
{
   checkParsed();
   if (mDisposition == 0)
   {
      mDisposition = new Content_Disposition_Header::Type;
   }
   return *mDisposition;
}

ParserContainer<Content_Language_MultiHeader::Type>&
Contents::header(const Content_Language_MultiHeader& headerType) const 
{
   checkParsed();
   if (mLanguages == 0)
   {
      mLanguages = new ParserContainer<Content_Language_MultiHeader::Type>;
   }
   return *mLanguages;
}

MIME_Transfer_Encoding_Header::Type&
Contents::header(const MIME_Transfer_Encoding_Header& headerType) const
{
   checkParsed();
   if (mTransferEncoding == 0)
   {
      mTransferEncoding = new MIME_Transfer_Encoding_Header::Type;
   }
   return *mTransferEncoding;
}

MIME_Description_Header::Type&
Contents::header(const MIME_Description_Header& headerType) const
{
   checkParsed();
   if (mDescription == 0)
   {
      mDescription = new MIME_Description_Header::Type;
   }
   return *mDescription;
}

MIME_Id_Header::Type&
Contents::header(const MIME_Id_Header& headerType) const
{
   checkParsed();
   if (mId == 0)
   {
      mId = new MIME_Id_Header::Type;
   }
   return *mId;
}

// !dlb! headers except Content-Disposition may contain (comments)
void
Contents::parseHeaders(ParseBuffer& pb)
{
   if (mHeadersFromMessage)
   {
      return;
   }

#if 1
   const char* start = pb.position();
   Data all( start, pb.end()-start);
   DebugLog(<< "Contents::parseHeaders" << all.escaped() );
#endif

   Data headerName;

   while ( !pb.eof() )
   {
      if ( *pb.position() == Symbols::CR[0] )
      {
         if ( *(pb.position()+1) == Symbols::LF[0] )
         {
            break;
         }
      }
            
      const char* anchor = pb.skipWhitespace();
      pb.skipToOneOf(Symbols::COLON, ParseBuffer::Whitespace);
      pb.data(headerName, anchor);

      pb.skipWhitespace();
      pb.skipChar(Symbols::COLON[0]);
      anchor = pb.skipWhitespace();
      pb.skipToTermCRLF();

      DebugLog(<< "Next header: <" << Data(anchor, pb.position() - anchor) << ">");

      Headers::Type type = Headers::getType(headerName.data(), headerName.size());
      ParseBuffer subPb(anchor, pb.position() - anchor);

      switch (type)
      {
         case Headers::Content_Type :
         {
            // already set
            break;
         }
         case Headers::Content_Disposition :
         {
            mDisposition = new Content_Disposition_Header::Type;
            mDisposition->parse(subPb);
            break;
         }
         // !dlb! not sure this ever happens?
         case Headers::Content_Language :
         {
            if (mLanguages == 0)
            {
               mLanguages = new ParserContainer<Content_Language_MultiHeader::Type>;
            }

            subPb.skipWhitespace();
            while (*subPb.position() != Symbols::COMMA[0])
            {
               Content_Language_MultiHeader::Type tmp;
               header(h_ContentLanguages).push_back(tmp);
               header(h_ContentLanguages).back().parse(subPb);
               subPb.skipLWS();
            }
         }
         default :
         {
            if (isEqualNoCase(headerName, "Content-Transfer-Encoding"))
            {
               mTransferEncoding = new Token();
               mTransferEncoding->parse(subPb);
            }
            else if (isEqualNoCase(headerName, "Content-Description"))
            {
               mDescription = new StringCategory();
               mDescription->parse(subPb);
            }
            else if (isEqualNoCase(headerName, "Content-Id"))
            {
               mId = new Token();
               mId->parse(subPb);
            }
            else if (isEqualNoCase(headerName, "MIME-Version"))
            {
               subPb.skipWhitespace();
               if (*subPb.position() == Symbols::LPAREN[0])
               {
                  subPb.skipToEndQuote(Symbols::RPAREN[0]);
                  subPb.skipChar(Symbols::RPAREN[0]);
               }
               mVersion = subPb.integer();

               if (*subPb.position() == Symbols::LPAREN[0])
               {
                  subPb.skipToEndQuote(Symbols::RPAREN[0]);
                  subPb.skipChar(Symbols::RPAREN[0]);
               }
               subPb.skipChar(Symbols::PERIOD[0]);
               
               if (*subPb.position() == Symbols::LPAREN[0])
               {
                  subPb.skipToEndQuote(Symbols::RPAREN[0]);
                  subPb.skipChar(Symbols::RPAREN[0]);
               }
               
               mMinorVersion = subPb.integer();
            }
            else
            {
               // add to application headers someday
               cerr << "Unknown MIME Content- header: " << headerName << endl;
               ErrLog(<< "Unknown MIME Content- header: " << headerName);
               assert(false);
            }
         }
      }
      pb.skipChars(Symbols::CRLF);
   }
   // !dlb! skipLWS first?
   pb.skipChars(Symbols::CRLF);
   DebugLog(<< "Content::parseHeaders post-headers <" << pb.position() << ">");
}

ostream&
Contents::encodeHeaders(ostream& str) const
{
   if (mHeadersFromMessage)
   {
      return str;
   }

   if (mVersion != 1 || mMinorVersion != 0)
   {
      str << "MIME-Version" << Symbols::COLON[0] << Symbols::SPACE[0]
          << mVersion << Symbols::PERIOD[0] << mMinorVersion 
          << Symbols::CRLF;
   }

   str << "Content-Type" << Symbols::COLON[0] << Symbols::SPACE[0]
       << mType 
       << Symbols::CRLF;

   if (exists(h_ContentDisposition))
   {
      str <<  "Content-Disposition" << Symbols::COLON[0] << Symbols::SPACE[0];

      header(h_ContentDisposition).encode(str);
      str << Symbols::CRLF;
   }

   if (exists(h_ContentLanguages))
   {
      str <<  "Content-Languages" << Symbols::COLON[0] << Symbols::SPACE[0];
      
      for (ParserContainer<Content_Language_MultiHeader::Type>::iterator 
              i = header(h_ContentLanguages).begin();
           i != header(h_ContentLanguages).end(); i++)
      {
         i->encode(str);
         str << Symbols::CRLF;
      }
   }

   if (mTransferEncoding)
   {
      str << "Content-Transfer-Encoding" << Symbols::COLON[0] << Symbols::SPACE[0]
          << *mTransferEncoding
          << Symbols::CRLF;
   }

   if (mId)
   {
      str << "Content-Id" << Symbols::COLON[0] << Symbols::SPACE[0]
          << *mId
          << Symbols::CRLF;
   }

   if (mDescription)
   {
      str << "Content-Description" << Symbols::COLON[0] << Symbols::SPACE[0]
          << *mDescription
          << Symbols::CRLF;
   }

   str << Symbols::CRLF;
   return str;
}


Data
Contents::getBodyData() const 
{
   ErrLog( << "Need to implement getBodyData function for " << getType() );
   assert(0);
   return Data::Empty;
}
