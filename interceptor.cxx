#include "interceptor.hxx"
#include <com/sun/star/task/XJobListener.hpp>
#include <com/sun/star/ui/ContextMenuInterceptorAction.hpp>
#include <com/sun/star/ui/XContextMenuInterception.hpp>
#include <com/sun/star/ui/ContextMenuExecuteEvent.hpp>
#include <com/sun/star/ui/ActionTriggerSeparatorType.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustring.hxx>

#include <cstdint>
#include <map>

using rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::system;
using namespace com::sun::star::frame;
using namespace com::sun::star::text;
using namespace com::sun::star::table;
using namespace com::sun::star::container;
using namespace com::sun::star::ui;
using namespace css;
using com::sun::star::beans::NamedValue;
using com::sun::star::beans::XPropertySet;
using com::sun::star::task::XJobListener;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::lang::XMultiServiceFactory;



// This is the service name an Add-On has to implement
#define SERVICE_NAME "com.sun.star.task.AsyncJob"

void SelectedTextSearch( const Reference< XFrame > &rxFrame );


// Helper functions for the implementation of UNO component interfaces.
OUString RegisterInterceptorJobImpl_getImplementationName()
throw (RuntimeException)
{
    return OUString ( IMPLEMENTATION_NAME );
}

Sequence< OUString > SAL_CALL RegisterInterceptorJobImpl_getSupportedServiceNames()
throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( SERVICE_NAME );
    return aRet;
}

Reference< XInterface > SAL_CALL RegisterInterceptorJobImpl_createInstance( const Reference< XComponentContext > & rContext)
    throw( Exception )
{
    return (cppu::OWeakObject*) new RegisterInterceptorJobImpl( rContext );
}

// Implementation of the recommended/mandatory interfaces of a UNO component.
// XServiceInfo
OUString SAL_CALL RegisterInterceptorJobImpl::getImplementationName()
    throw (RuntimeException)
{
    return RegisterInterceptorJobImpl_getImplementationName();
}

sal_Bool SAL_CALL RegisterInterceptorJobImpl::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL RegisterInterceptorJobImpl::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return RegisterInterceptorJobImpl_getSupportedServiceNames();
}

// XAsyncJob method implementations

void SAL_CALL RegisterInterceptorJobImpl::executeAsync( const Sequence<NamedValue>& rArgs,
					  const Reference<XJobListener>& rxListener )
    throw(IllegalArgumentException, RuntimeException)
{

    sal_Int32 nNumNVs = rArgs.getLength();
    Sequence<NamedValue> aEnvironment;
    for ( sal_Int32 nIdx = 0; nIdx < nNumNVs; ++nIdx )
    {
	if ( rArgs[nIdx].Name.equalsAscii("Environment") )
	{
	    rArgs[nIdx].Value >>= aEnvironment;
	    break;
	}
    }

    if ( aEnvironment.hasElements() )
    {
	sal_Int32 nNumEnvEntries = aEnvironment.getLength();
	Reference< XFrame > xFrame;
	OUString aEnvType, aEventName;
	for ( sal_Int32 nIdx = 0; nIdx < nNumEnvEntries; ++nIdx )
	{
	    if ( aEnvironment[nIdx].Name.equalsAscii("Frame") )
		aEnvironment[nIdx].Value >>= xFrame;
	    else if ( aEnvironment[nIdx].Name.equalsAscii("EnvType") )
		aEnvironment[nIdx].Value >>= aEnvType;
	    else if ( aEnvironment[nIdx].Name.equalsAscii("EventName") )
		aEnvironment[nIdx].Value >>= aEventName;
	}
	printf( "DEBUG>>> xFrame = %p\n", xFrame.get()); fflush(stdout);
	if ( aEnvType.equalsAscii("DISPATCH") && xFrame.is() )
	{
		if (  aEventName.equalsAscii("onEnableInterceptClick1") )
		{
			sal_Int32 nNumCalls = 0;
			Reference< XContextMenuInterceptor > xInterceptor = getInterceptor( xFrame, nNumCalls );
			if ( nNumCalls > 1 )
			{
				printf("DEBUG>>> Interceptor is already enabled\n");
				fflush(stdout);
			}
			else
			{
				Reference< XController > xController = xFrame->getController();
				if ( xController.is() )
				{
					Reference< XContextMenuInterception > xContextMenuInterception( xController, UNO_QUERY );
					if ( xContextMenuInterception.is() )
					{
						xContextMenuInterception->registerContextMenuInterceptor( xInterceptor );
					}
				}
			}
		}
		else if ( aEventName.equalsAscii("onIncrementClick") ) {
                SelectedTextSearch( xFrame );
	    }
	}
    }

    Sequence<NamedValue> aReturn;
    rxListener->jobFinished( Reference<com::sun::star::task::XAsyncJob>(this), makeAny(aReturn));
    
}

Reference< XContextMenuInterceptor >& RegisterInterceptorJobImpl::getInterceptor( const Reference<XFrame>& xFrame, sal_Int32& rCalls )
{
    static Reference< XContextMenuInterceptor > xInterceptor;
    static std::map<std::uintptr_t, sal_Int32> aFrame2Calls;
    if ( !xInterceptor.is() )
	xInterceptor = Reference< XContextMenuInterceptor >( (cppu::OWeakObject*) new ContextMenuInterceptorImpl(), UNO_QUERY );

    std::uintptr_t nFramePtrVal = reinterpret_cast<std::uintptr_t>( xFrame.get() );
    auto aItr = aFrame2Calls.find( nFramePtrVal );
    if ( aItr != aFrame2Calls.end() )
	rCalls = ++(aItr->second);
    else
    {
	aFrame2Calls[ nFramePtrVal ] = 1;
	rCalls = 1;
    }
    return xInterceptor;
}


void logError(const char* pStr)
{
    printf(pStr);
    fflush(stdout);
}

Reference<XTextViewCursor> getXTextViewCursor( const Reference<XModel >& xModel )
{
    Reference<XController > xController = xModel->getCurrentController();
    Reference<XTextViewCursorSupplier > xTextViewCursorSupp( xController,UNO_QUERY_THROW );
    Reference<XTextViewCursor> xTextViewCursor = xTextViewCursorSupp->getViewCursor();
    return xTextViewCursor;
}

void SelectedTextSearch( const Reference< XFrame > &rxFrame )
{
    if ( !rxFrame.is() )
    {
	return;
    }
    
    Reference< XController > xCtrl = rxFrame->getController();
    if ( !xCtrl.is() )
    {
	return;
    }

    Reference< XModel > xModel = xCtrl->getModel();
    if ( !xModel.is() )
    {
	return;
    }
	Reference<XTextRange> xTextRange;
	Reference<XTextContent> xTextContent( xModel->getCurrentSelection(),UNO_QUERY );
    if ( !xTextContent.is() ) // Multimarked
    {
		Reference<XIndexAccess> xIndexAccess( xModel->getCurrentSelection(),UNO_QUERY );
		if( xIndexAccess.is() )
		{
			xTextContent.set( xIndexAccess->getByIndex(0),UNO_QUERY );
		}
    }
	if( xTextContent.is() )
		xTextRange = xTextContent->getAnchor();
	if( !xTextRange.is() )
         xTextRange.set( getXTextViewCursor( xModel ),UNO_QUERY_THROW );
  
    Reference< XText > xText= xTextRange->getText();
    OUString stringText = xTextRange->getString() ;
	Reference<XSystemShellExecute > xSystemShellExecute(system::SystemShellExecute::create(comphelper::getProcessComponentContext()));
    if( stringText.isEmpty()) {}
    else
    {
		OUString URL = "https://wikipedia.org/wiki/"+stringText;
		xSystemShellExecute->execute(URL, OUString(),SystemShellExecuteFlags::URIS_ONLY );

    }
    
}




ContextMenuInterceptorAction SAL_CALL ContextMenuInterceptorImpl::notifyContextMenuExecute(
    const ContextMenuExecuteEvent& rEvent )
    throw ( RuntimeException )
{

    try {
	Reference< XIndexContainer > xContextMenu = rEvent.ActionTriggerContainer;
	if ( !xContextMenu.is() )
	{
	    return ContextMenuInterceptorAction_IGNORED;
	}
	Reference< XMultiServiceFactory > xMenuElementFactory( xContextMenu, UNO_QUERY );
	if ( !xMenuElementFactory.is() )
	{
	    return ContextMenuInterceptorAction_IGNORED;
	}

	Reference< XPropertySet > xSeparator( xMenuElementFactory->createInstance( "com.sun.star.ui.ActionTriggerSeparator" ), UNO_QUERY );
	if ( !xSeparator.is() )
	{
	    return ContextMenuInterceptorAction_IGNORED;
	}

	xSeparator->setPropertyValue( "SeparatorType", makeAny( ActionTriggerSeparatorType::LINE ) );
	
	Reference< XPropertySet > xMenuEntry( xMenuElementFactory->createInstance( "com.sun.star.ui.ActionTrigger" ), UNO_QUERY );
	if ( !xMenuEntry.is() )
	{
	    return ContextMenuInterceptorAction_IGNORED;
	}

	xMenuEntry->setPropertyValue( "Text", makeAny(OUString("Search")));
	xMenuEntry->setPropertyValue( "CommandURL", makeAny( OUString("vnd.sun.star.job:event=onIncrementClick") ) );

	sal_Int32 nCount = xContextMenu->getCount();
	sal_Int32 nIdx = nCount;
	xContextMenu->insertByIndex( nIdx++, makeAny( xSeparator ) );
	xContextMenu->insertByIndex( nIdx++, makeAny( xMenuEntry ) );

	return ContextMenuInterceptorAction_CONTINUE_MODIFIED;
    }
    catch ( Exception& e )
    {}

    return ContextMenuInterceptorAction_IGNORED;
}

