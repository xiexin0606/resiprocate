#include "InviteSessionCreator.hxx"
#include "resiprocate/SdpContents.hxx"

using namespace resip;

InviteSessionCreator::InviteSessionCreator(DialogUsageManager& dum, 
                                           const NameAddr& target, 
                                           const NameAddr& from,
                                           const SdpContents* initial, 
                                           ServerSubscriptionHandle serverSub)
   : BaseCreator(dum),
     mState(Initialized),
     mInitialOffer(static_cast<SdpContents*>(initial->clone())),
     mServerSub(serverSub)
{
   makeInitialRequest(target, from, INVITE);
   getLastRequest().setContents(mInitialOffer);
}

InviteSessionCreator::~InviteSessionCreator()
{
	delete mInitialOffer;
}

void
InviteSessionCreator::end()
{
   assert(0);
}

void
InviteSessionCreator::dispatch(const SipMessage& msg)
{
   // !jf! does this have to do with CANCELing all of the branches associated
   // with a single invite request
}

const SdpContents* 
InviteSessionCreator::getInitialOffer() const
{
   return mInitialOffer;
}

