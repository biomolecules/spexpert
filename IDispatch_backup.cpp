    QAxObject * axSpectrum = new QAxObject(this);
    axSpectrum->setControl("{d23922f2-36bd-401e-9dc3-db1924be7a2d}");
//    QList<QVariant> iSpectrum;
//    for (int ii = 0; ii < iFrames * 2048; ++ii)
//        iSpectrum << 0L;
//    qDebug() << blOut << "Size = " << spectrum.size();
    long iFrm, iX, iY;
    iFrm = 0;
//    QList<QVariant> params;
//    params << QVariant(&iSpectrum) << iFrm << iX << iY;
//    bool blOut = axSpectrum->dynamicCall("GetSpectrum(QList<long>&, long&, long&, long&)", params).toBool();
//    winSpec->GetSpectrum(spectrum, iFrm, iX, iY);

//    IUnknown *iface = 0;
    IDispatch *iface = 0;
    axSpectrum->queryInterface(IID_IUnknown, (void**)&iface);
    if (iface) {
       DISPID dispid;
//       DISPID dispid2;
       OLECHAR * dispname = L"GetSpectrum";
//       iface->GetIDsOfNames(IID_NULL, &dispname, 1, LOCALE_USER_DEFAULT, &dispid);
//       OLECHAR * dispname = L"SetAcqParams";
//       OLECHAR * dispname2 = L"GetAcqParams";
       iface->GetIDsOfNames(IID_NULL, &dispname, 1, LOCALE_USER_DEFAULT, &dispid);
//       iface->GetIDsOfNames(IID_NULL, &dispname2, 1, LOCALE_USER_DEFAULT, &dispid2);
       qDebug() << hex << (int)dispid;
//       int axes0[3]={0,1,2};
       SAFEARRAY *spectrum = NULL;
       SAFEARRAYBOUND axesbound[1];
       axesbound[0].lLbound = 0;
       axesbound[0].cElements = iFrames * 2048;
       spectrum = SafeArrayCreate(VT_I4, 1, axesbound);
       if (spectrum)
           qDebug() << "Pole vytvoreno!";
       HRESULT hrAlloc = SafeArrayAllocData(spectrum);
       if (hrAlloc == S_OK)
           qDebug() << "Pole alokovano";
       SAFEARRAY **pSpectrum = &spectrum;
//       SafeArrayAccessData(spectrum, (void**)pSpectrum);
//       long arSpectrum[iFrames * 2048] = {};
//       for (long ii = 0; ii < iFrames * 2048; ++ii)
//               SafeArrayPutElement(spectrum,&ii,arSpectrum+ii);
//       for(long index=0;index<3;index++)
//       {
//       ::SafeArrayPutElement(spectrum,&index,axes0+index);
//       }

       VARIANTARG v[4];
//       v[3].plVal = &iY; //number of axes
//       v[3].vt = VT_I4 | VT_BYREF;
//       v[2].plVal = &iX; //number of axes
//       v[2].vt = VT_I4 | VT_BYREF;
//       v[1].plVal = &iFrm; //number of axes
//       v[1].vt = VT_I4 | VT_BYREF;
//       v[0].pparray = pSpectrum;
//       v[0].vt = VT_ARRAY | VT_I4 | VT_BYREF;

       VARIANT_BOOL blStatus;

       v[3].pboolVal = &blStatus; //number of axes
       v[3].vt = VT_BOOL | VT_BYREF;
       v[2].plVal = &iY; //number of axes
       v[2].vt = VT_I4 | VT_BYREF;
       v[1].plVal = &iX; //number of axes
       v[1].vt = VT_I4 | VT_BYREF;
       v[0].plVal = &iFrm; //number of axes
       v[0].vt = VT_I4 | VT_BYREF;

//       BSTR fn = SysAllocString(L"D:\\users\\Lumik\\Desktop\\Temp\\WinSpecTest\\temp.spe");
//       VARIANTARG v[3];
//       v[0].dblVal = dblExposure;
//       v[0].vt = VT_R8;
//       v[1].intVal = iAccums;
//       v[1].vt = VT_INT;
//       v[2].intVal = iFrames;
//       v[2].vt = VT_INT;
//       VARIANTARG v2[3];
//       v2[0].pdblVal = &dblExposure;
//       v2[0].vt = VT_R8 | VT_BYREF;
//       v2[1].pintVal = &iAccums;
//       v2[1].vt = VT_INT | VT_BYREF;
//       v2[2].pintVal = &iFrm;
//       v2[2].vt = VT_INT | VT_BYREF;
       DISPPARAMS params = {v, NULL, 4, 0};
//       DISPPARAMS params = {v, NULL, 3, 0};
//       DISPPARAMS params2 = {v2, NULL, 3, 0};
       VARIANT result;
       qDebug() << "Ziskavam pole.";
       HRESULT hr = iface->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params , &result, NULL, NULL);
//       HRESULT hr2 = iface->Invoke(dispid2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params2 ,NULL, NULL, NULL);
       qDebug() << "Mam pole: ";
       switch (hr)
       {
       case S_OK: qDebug() << "OK"; break;
       case DISP_E_BADPARAMCOUNT: qDebug() << "DISP_E_BADPARAMCOUNT"; break;
       case DISP_E_BADVARTYPE: qDebug() << "DISP_E_BADVARTYPE"; break;
       case DISP_E_EXCEPTION: qDebug() << "DISP_E_EXCEPTION"; break;
       case DISP_E_MEMBERNOTFOUND: qDebug() << "DISP_E_MEMBERNOTFOUND"; break;
       case DISP_E_NONAMEDARGS: qDebug() << "DISP_E_NONAMEDARGS"; break;
       case DISP_E_OVERFLOW: qDebug() << "DISP_E_OVERFLOW"; break;
       case DISP_E_PARAMNOTFOUND: qDebug() << "DISP_E_PARAMNOTFOUND"; break;
       case DISP_E_TYPEMISMATCH: qDebug() << "DISP_E_TYPEMISMATCH"; break;
       case DISP_E_UNKNOWNINTERFACE: qDebug() << "DISP_E_UNKNOWNINTERFACE"; break;
       case DISP_E_UNKNOWNLCID: qDebug() << "DISP_E_UNKNOWNLCID"; break;
       case DISP_E_PARAMNOTOPTIONAL: qDebug() << "DISP_E_PARAMNOTOPTIONAL"; break;
       default: qDebug() << "Not known!!!" << (int)hr;
       }
       qDebug() << "Framu: " << iFrm;
//       int ii = 0;
//       while ((winSpec->Running()))
//           qDebug() << "Running: " << ++ii << "; Frame: " << winSpec->GetFrame();
//       qDebug("Finished");

       iface->Release();
