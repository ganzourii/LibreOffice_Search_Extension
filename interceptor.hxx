#ifndef INCO_NIOCS_TEST_INTERCEPTOREXTENSION_HXX
#define INCO_NIOCS_TEST_INTERCEPTOREXTENSION_HXX

#include <stdio.h>
#include <com/sun/star/task/XAsyncJob.hpp>
#include <com/sun/star/ui/XContextMenuInterceptor.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>

#define IMPLEMENTATION_NAME "inco.niocs.test.RegisterInterceptorJobImpl"

namespace com
{
    namespace sun
    {
        namespace star
        {
	    namespace frame
	    {
		class XFrame;
	    }
            namespace uno
            {
                class XComponentContext;
            }
	    namespace beans
	    {
		struct NamedValue;
	    }
	    namespace task
	    {
		class XJobListener;
	    }
	    namespace ui
	    {
		enum ContextMenuInterceptorAction;
		struct ContextMenuExecuteEvent;
	    }
        }
    }
}


// XContextMenuInterceptor implementer class
class ContextMenuInterceptorImpl : public cppu::WeakImplHelper1< ::com::sun::star::ui::XContextMenuInterceptor >
{

public:

    ContextMenuInterceptorImpl()
    {
	printf("DEBUG>>> Created ContextMenuInterceptorImpl object : %p\n", this); fflush(stdout);
    }

    virtual ::com::sun::star::ui::ContextMenuInterceptorAction SAL_CALL notifyContextMenuExecute(
	const ::com::sun::star::ui::ContextMenuExecuteEvent& rEvent )
	throw ( ::com::sun::star::uno::RuntimeException ); 
};

class RegisterInterceptorJobImpl : public cppu::WeakImplHelper2
<
    com::sun::star::task::XAsyncJob,
    com::sun::star::lang::XServiceInfo
>
{
    
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxContext;

public:

    RegisterInterceptorJobImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rxContext )
        : mxContext( rxContext )
    {
	printf("DEBUG>>> Created RegisterInterceptorJobImpl object : %p\n", this); fflush(stdout);
    }

    // XAsyncJob methods
    virtual void SAL_CALL executeAsync( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rArgs,
					const ::com::sun::star::uno::Reference< ::com::sun::star::task::XJobListener >& rxListener )
	throw(::com::sun::star::lang::IllegalArgumentException,
	      ::com::sun::star::uno::RuntimeException) override;

    // XServiceInfo methods
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& aServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

private:
    static ::com::sun::star::uno::Reference< ::com::sun::star::ui::XContextMenuInterceptor >& getInterceptor(
	const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
	sal_Int32& rCalls );
};

::rtl::OUString RegisterInterceptorJobImpl_getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL RegisterInterceptorJobImpl_supportsService( const ::rtl::OUString& ServiceName )
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL RegisterInterceptorJobImpl_getSupportedServiceNames()
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL RegisterInterceptorJobImpl_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & rContext)
    throw ( ::com::sun::star::uno::Exception );




#endif
