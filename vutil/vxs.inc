/* This file is part of the "version" CPAN distribution.  Please avoid
   editing it in the perl core. */

#ifdef PERL_CORE
#  define VXS_CLASS "version"
#else
#  define VXS_CLASS "version::vxs"
#endif

#ifdef VXS_XSUB_DETAILS
#  ifdef PERL_CORE
    {"UNIVERSAL::VERSION", XS_UNIVERSAL_VERSION, NULL},
#  else
    {VXS_CLASS "::_VERSION", XS_UNIVERSAL_VERSION, NULL},
#  endif
    {VXS_CLASS "::()", XS_version_noop, NULL},
    {VXS_CLASS "::new", XS_version_new, NULL},
    {VXS_CLASS "::parse", XS_version_new, NULL},
    {VXS_CLASS "::(\"\"", XS_version_stringify, NULL},
    {VXS_CLASS "::stringify", XS_version_stringify, NULL},
    {VXS_CLASS "::(0+", XS_version_numify, NULL},
    {VXS_CLASS "::numify", XS_version_numify, NULL},
    {VXS_CLASS "::normal", XS_version_normal, NULL},
    {VXS_CLASS "::(cmp", XS_version_vcmp, NULL},
    {VXS_CLASS "::(<=>", XS_version_vcmp, NULL},
#  ifdef PERL_CORE
    {VXS_CLASS "::vcmp", XS_version_vcmp, NULL},
#  else
    {VXS_CLASS "::VCMP", XS_version_vcmp, NULL},
#  endif
    {VXS_CLASS "::(bool", XS_version_boolean, NULL},
    {VXS_CLASS "::boolean", XS_version_boolean, NULL},
    {VXS_CLASS "::(+", XS_version_noop, NULL},
    {VXS_CLASS "::(-", XS_version_noop, NULL},
    {VXS_CLASS "::(*", XS_version_noop, NULL},
    {VXS_CLASS "::(/", XS_version_noop, NULL},
    {VXS_CLASS "::(+=", XS_version_noop, NULL},
    {VXS_CLASS "::(-=", XS_version_noop, NULL},
    {VXS_CLASS "::(*=", XS_version_noop, NULL},
    {VXS_CLASS "::(/=", XS_version_noop, NULL},
    {VXS_CLASS "::(abs", XS_version_noop, NULL},
    {VXS_CLASS "::(nomethod", XS_version_noop, NULL},
    {VXS_CLASS "::noop", XS_version_noop, NULL},
    {VXS_CLASS "::is_alpha", XS_version_is_alpha, NULL},
    {VXS_CLASS "::qv", XS_version_qv, NULL},
    {VXS_CLASS "::declare", XS_version_qv, NULL},
    {VXS_CLASS "::is_qv", XS_version_is_qv, NULL},
#else

#ifndef dVAR
#  define dVAR
#endif

#ifdef HvNAME_HEK
typedef HEK HVNAME;
#  ifndef HEKf
#    define HEKfARG(arg)	((void*)(sv_2mortal(newSVhek(arg))))
#    define HEKf		SVf
#  endif
#else
typedef char HVNAME;
#  define HvNAME_HEK	HvNAME_get
#  define HEKfARG(arg)	arg
#  define HEKf		"s"
#endif

XS(XS_UNIVERSAL_VERSION)
{
    dVAR;
    dXSARGS;
    HV *pkg;
    GV **gvp;
    GV *gv;
    SV *ret;
    SV *sv;
    const char *undef;
    PERL_UNUSED_ARG(cv);

    if (items < 1)
       Perl_croak(aTHX_ "Usage: UNIVERSAL::VERSION(sv, ...)");

    sv = ST(0);

    if (SvROK(sv)) {
        sv = (SV*)SvRV(sv);
        if (!SvOBJECT(sv))
            Perl_croak(aTHX_ "Cannot find version of an unblessed reference");
        pkg = SvSTASH(sv);
    }
    else {
        pkg = gv_stashsv(sv, FALSE);
    }

    gvp = pkg ? (GV**)hv_fetchs(pkg,"VERSION",FALSE) : Null(GV**);

    if (gvp && isGV(gv = *gvp) && (sv = GvSV(gv)) && SvOK(sv)) {
        sv = sv_mortalcopy(sv);
	if ( ! ISA_CLASS_OBJ(sv, "version::vxs"))
	    UPG_VERSION(sv, FALSE);
        undef = NULL;
    }
    else {
        sv = ret = &PL_sv_undef;
        undef = "(undef)";
    }

    if (items > 1) {
	SV *req = ST(1);

	if (undef) {
	    if (pkg) {
		const HVNAME* const name = HvNAME_HEK(pkg);
#if PERL_VERSION == 5
		Perl_croak(aTHX_ "%s version %s required--this is only version ",
			    name, SvPVx_nolen_const(req));
#else
		Perl_croak(aTHX_
			   "%"HEKf" does not define $%"HEKf
			   "::VERSION--version check failed",
			   HEKfARG(name), HEKfARG(name));
#endif
	    }
	    else {
#if PERL_VERSION >= 8
		Perl_croak(aTHX_
			     "%"SVf" defines neither package nor VERSION--version check failed",
			     (void*)(ST(0)) );
#else
		Perl_croak(aTHX_ "%s does not define $%s::VERSION--version check failed",
			   SvPVx_nolen_const(ST(0)),
			   SvPVx_nolen_const(ST(0)) );
#endif
	    }
	}

	if ( ! ISA_CLASS_OBJ(req, "version")) {
	    /* req may very well be R/O, so create a new object */
	    req = sv_2mortal( NEW_VERSION(req) );
	}

	if ( VCMP( req, sv ) > 0 ) {
	    if ( hv_exists(MUTABLE_HV(SvRV(req)), "qv", 2 ) ) {
		req = VNORMAL(req);
		sv  = VNORMAL(sv);
	    }
	    else {
		req = VSTRINGIFY(req);
		sv  = VSTRINGIFY(sv);
	    }
	    Perl_croak(aTHX_ "%"HEKf" version %"SVf" required--"
		"this is only version %"SVf"", HEKfARG(HvNAME_HEK(pkg)),
		SVfARG(sv_2mortal(req)),
		SVfARG(sv_2mortal(sv)));
	}
    }
    ST(0) = ret;

    /* if the package's $VERSION is not undef, it is upgraded to be a version object */
    if (ISA_CLASS_OBJ(sv, "version")) {
	ST(0) = sv_2mortal(VSTRINGIFY(sv));
    } else {
	ST(0) = sv;
    }

    XSRETURN(1);
}

XS(XS_version_new)
{
    dVAR;
    dXSARGS;
    PERL_UNUSED_VAR(cv);
    SV *vs = ST(1);
    SV *rv;
    const char * classname = "";
    SP -= items;

    /* Just in case this is something like a tied hash */
    SvGETMAGIC(vs);

    if (items > 3 || items == 0)
        Perl_croak(aTHX_ "Usage: version::new(class, version)");

    if ( items == 1 || ! SvOK(vs) ) { /* no param or explicit undef */
        /* create empty object */
        vs = sv_newmortal();
        sv_setpvs(vs,"undef");
    }
    else if (items == 3 ) {
        vs = sv_newmortal();
        sv_setpvf(vs,"v%s",SvPV_nolen_const(ST(2)));
    }
    classname =
	sv_isobject(ST(0)) /* get the class if called as an object method */
	    ? HvNAME_get(SvSTASH(SvRV(ST(0))))
	    : (char *)SvPV_nolen(ST(0));

    rv = NEW_VERSION(vs);
    if ( strcmp(classname,"version::vxs") != 0 ) /* inherited new() */
#if PERL_VERSION == 5
        sv_bless(rv, gv_stashpv((char *)classname, GV_ADD));
#else
        sv_bless(rv, gv_stashpv(classname, GV_ADD));
#endif

    mPUSHs(rv);
    PUTBACK;
    return;
}

#define VTYPECHECK(var, val, varname) \
    STMT_START {							\
	if (ISA_CLASS_OBJ(val, "version")) {				\
	    (var) = SvRV(val);						\
	}								\
	else								\
	    Perl_croak(aTHX_ varname " is not of type version");	\
    } STMT_END

XS(XS_version_stringify)
{
     dVAR;
     dXSARGS;
     if (items < 1)
	 croak_xs_usage(cv, "lobj, ...");
     SP -= items;
     {
	  SV *	lobj;
	  VTYPECHECK(lobj, ST(0), "lobj");

	  mPUSHs(VSTRINGIFY(lobj));

	  PUTBACK;
	  return;
     }
}

XS(XS_version_numify)
{
     dVAR;
     dXSARGS;
     if (items < 1)
	 croak_xs_usage(cv, "lobj, ...");
     SP -= items;
     {
	  SV *	lobj;
	  VTYPECHECK(lobj, ST(0), "lobj");
	  mPUSHs(VNUMIFY(lobj));
	  PUTBACK;
	  return;
     }
}

XS(XS_version_normal)
{
     dVAR;
     dXSARGS;
     if (items != 1)
	 croak_xs_usage(cv, "ver");
     SP -= items;
     {
	  SV *	ver;
	  VTYPECHECK(ver, ST(0), "ver");

	  mPUSHs(VNORMAL(ver));

	  PUTBACK;
	  return;
     }
}

XS(XS_version_vcmp)
{
     dVAR;
     dXSARGS;
     if (items < 1)
	 croak_xs_usage(cv, "lobj, ...");
     SP -= items;
     {
	  SV *	lobj;
	  VTYPECHECK(lobj, ST(0), "lobj");
	  {
	       SV	*rs;
	       SV	*rvs;
	       SV * robj = ST(1);
	       const IV	 swap = (IV)SvIV(ST(2));

	       if ( !ISA_CLASS_OBJ(robj, "version::vxs") )
	       {
		    robj = NEW_VERSION(SvOK(robj) ? robj : newSVpvs_flags("0", SVs_TEMP));
		    sv_2mortal(robj);
	       }
	       rvs = SvRV(robj);

	       if ( swap )
	       {
		    rs = newSViv(VCMP(rvs,lobj));
	       }
	       else
	       {
		    rs = newSViv(VCMP(lobj,rvs));
	       }

	       mPUSHs(rs);
	  }

	  PUTBACK;
	  return;
     }
}

XS(XS_version_boolean)
{
    dVAR;
    dXSARGS;
    SV *lobj;
    if (items < 1)
	croak_xs_usage(cv, "lobj, ...");
    SP -= items;
    VTYPECHECK(lobj, ST(0), "lobj");
    {
	SV * const rs =
	    newSViv( VCMP(lobj,
			  sv_2mortal(NEW_VERSION(
					sv_2mortal(newSVpvs("0"))
				    ))
			 )
		   );
	mPUSHs(rs);
	PUTBACK;
	return;
    }
}

XS(XS_version_noop)
{
    dVAR;
    dXSARGS;
    if (items < 1)
	croak_xs_usage(cv, "lobj, ...");
    if (ISA_CLASS_OBJ(ST(0), "version"))
	Perl_croak(aTHX_ "operation not supported with version object");
    else
	Perl_croak(aTHX_ "lobj is not of type version");
    XSRETURN_EMPTY;
}

XS(XS_version_is_alpha)
{
    dVAR;
    dXSARGS;
    if (items != 1)
	croak_xs_usage(cv, "lobj");
    SP -= items;
    {
	SV *lobj;
	VTYPECHECK(lobj, ST(0), "lobj");
	if ( hv_exists(MUTABLE_HV(SvRV(lobj)), "alpha", 5 ) )
	    XSRETURN_YES;
	else
	    XSRETURN_NO;
	PUTBACK;
	return;
    }
}

XS(XS_version_qv)
{
    dVAR;
    dXSARGS;
    PERL_UNUSED_ARG(cv);
    SP -= items;
    {
	SV * ver = ST(0);
	SV * rv;
        const char * classname = "";
        if ( items == 2 ) {
	    SvGETMAGIC(ST(1));
	    if (SvOK(ST(1))) {
		ver = ST(1);
	    }
	    else {
		Perl_croak(aTHX_ "Invalid version format (version required)");
	    }
	}
	if ( !SvVOK(ver) ) { /* not already a v-string */
	    rv = sv_newmortal();
	    sv_setsv(rv,ver); /* make a duplicate */
	    UPG_VERSION(rv, TRUE);
	} else {
	    rv = sv_2mortal(NEW_VERSION(ver));
	}
	classname =
	    sv_isobject(ST(0))
		/* get the class if called as an object method */
		? HvNAME_get(SvSTASH(SvRV(ST(0))))
		: (char *)SvPV_nolen(ST(0));
	if ( items == 2
                && strcmp(classname,"version") ) { /* inherited new() */
#if PERL_VERSION == 5
	    sv_bless(rv, gv_stashpv((char *)classname, GV_ADD));
#else
	    sv_bless(rv, gv_stashpv(classname, GV_ADD));
#endif
        }
	PUSHs(rv);
    }
    PUTBACK;
    return;
}

XS(XS_version_is_qv)
{
    dVAR;
    dXSARGS;
    if (items != 1)
	croak_xs_usage(cv, "lobj");
    SP -= items;
    {
	SV *lobj;
	VTYPECHECK(lobj, ST(0), "lobj");
	if ( hv_exists(MUTABLE_HV(SvRV(lobj)), "qv", 2 ) )
	    XSRETURN_YES;
	else
	    XSRETURN_NO;
	PUTBACK;
	return;
    }
}

#endif