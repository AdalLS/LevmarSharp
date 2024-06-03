// LevmarSharp.h
#include "levmar.h"
#include "malloc.h"
#define NULL 0
#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;

namespace LevmarSharp {
	/// <summary>
	/// A delegate for the method describing the functional relation required by the Levenberg-Marquardt algorithm.
	/// </summary>
	/// <param name="p">Initial parameter estimates.</param>
	/// <param name="hx">Measurement vector.</param>
	/// <param name="m">Parameter vector dimension.</param>
	/// <param name="n">Measurement vector dimension.</param>
	/// <param name="data">Additional data, passed uninterpreted to function.</param>
	public delegate void LevmarFunc(array<double>^ p, array<double>^ hx, int m, int n, IntPtr data);

	/// A delegate for the method evaluating the Jacobian required by the Levenberg-Marquardt algorithm.
	/// </summary>
	/// <param name="p">Initial parameter estimates.</param>
	/// <param name="j">The Jacobian.</param>
	/// <param name="m">Parameter vector dimension.</param>
	/// <param name="n">Measurement vector dimension.</param>
	/// <param name="data">Additional data, passed uninterpreted to function.</param>
	public delegate void LevmarJacf(array<double>^ p, array<double>^ j, int m, int n, IntPtr data);

	public ref class Levmar
	{
		[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
		delegate void LevmarFuncU(double *p, double *hx, int m, int n, void *data);
		[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
		delegate void LevmarJacfU(double *p, double *j, int m, int n, void *data);
		typedef void (*func)(double *p, double *hx, int m, int n, void *adata);
		typedef void (*jacf)(double *p, double *j, int m, int n, void *adata);

		int jacSize;
		int hxSize;
		int pSize;
		LevmarFunc^ functionM;
		LevmarJacf^ functionJ;

		void UnmanagedFuncWrapper(double *p, double *hx, int m, int n, void *data)
		{
			array<double>^ pM = gcnew array<double>(pSize);
			array<double>^ hxM = gcnew array<double>(hxSize);

			Marshal::Copy(IntPtr(p),pM, 0, pM->Length);
			Marshal::Copy(IntPtr(hx),hxM, 0, hxM->Length);
			IntPtr dataM = IntPtr(data);
			pin_ptr<double> pU = &pM[0];
			pin_ptr<double> hxU = &hxM[0];

			functionM(pM, hxM, m, n, dataM);

			Marshal::Copy(pM, 0, IntPtr(p), pM->Length);
			Marshal::Copy(hxM,0, IntPtr(hx), hxM->Length);
			data = dataM.ToPointer();
		}

		void UnmanagedJacfWrapper(double *p, double *j, int m, int n, void *data)
		{
			array<double>^ pM = gcnew array<double>(pSize);
			array<double>^ jM = gcnew array<double>(jacSize);

			Marshal::Copy(IntPtr(p),pM, 0, pM->Length);
			Marshal::Copy(IntPtr(j),jM, 0, jM->Length);
			IntPtr dataM = IntPtr(data);
			pin_ptr<double> pU = &pM[0];
			pin_ptr<double> hxU = &jM[0];

			functionJ(pM, jM, m, n, dataM);

			Marshal::Copy(pM, 0, IntPtr(p), pM->Length);
			Marshal::Copy(jM,0, IntPtr(j), jM->Length);
			data = dataM.ToPointer();
		}

	public:
		static int LmOptionsSize    	= 5; /* max(4, 5) */
		static int LmInfoSize   	 = 10;
		static int LmError			 = -1;
		static double LmInitMu    	 =1E-03;
		static double LmDiffDelta    = 1E-06;

		/// <summary>
		/// Secant version of the LevmarDer function: the Jacobian is approximated with
		/// the aid of finite differences.
		/// </summary>
		/// <param name="f">Delegate to the method expressing the functional relation describing measurements.</param>
		/// <param name="p">Initial parameter estimates. On output has the estimated solution.</param>
		/// <param name="x">Measurement vector. NULL implies a zero vector</param>
		/// <param name="m">Parameter vector dimension.</param>
		/// <param name="n">Measurement vector dimension.</param>
		/// <param name="itmax">Maximum number of iterations</param>
		/// <param name="opts">opts[0-4] = Minimum options. Set to NULL for defaults to be used.</param>
		/// <param name="info">Returns output information regarding the minimization. Set to NULL if don't care.</param>
		/// <returns>The number of iterations used. LmError (-1) if failed.</returns>
		int LevmarDif(LevmarFunc^ f, array<double> ^%p, array<double> ^x, int m, int n, int itmax,
			array<double>^ opts, array<double>^% info)
		{
			return LevmarDif(f, p,x,m,n,itmax,opts,info, IntPtr::Zero);
		}

		/// <summary>
		/// Initializes a new instance of the <see cref="Levmar"/> class.
		/// </summary>
		Levmar(){}
		///
		/// <summary>
		/// Secant version of the LevmarDer function: the Jacobian is approximated with
		/// the aid of finite differences.
		/// </summary>
		/// <param name="f">Delegate to the method expressing the functional relation describing measurements.</param>
		/// <param name="p">Initial parameter estimates. On output has the estimated solution.</param>
		/// <param name="x">Measurement vector. NULL implies a zero vector</param>
		/// <param name="m">Parameter vector dimension.</param>
		/// <param name="n">Measurement vector dimension.</param>
		/// <param name="itmax">Maximum number of iterations</param>
		/// <param name="opts">opts[0-4] = Minimum options. Set to NULL for defaults to be used.</param>
		/// <param name="info">Returns output information regarding the minimization. Set to NULL if don't care.</param>
		/// <param name="data">Additional data, passed uninterpreted to function.</param>
		/// <returns>The number of iterations used. LmError (-1) if failed.</returns>
		int LevmarDif(LevmarFunc^ f, array<double> ^%p, array<double> ^x, int m, int n, int itmax,
			array<double>^ opts, array<double>^% info, IntPtr data)
		{
			pin_ptr<double> xU = &x[0];
			pin_ptr<double> pU = &p[0];
			pin_ptr<double> optsU = &opts[0];
			pin_ptr<double> infoU = &info[0];

			functionM = f;
			pSize = p->Length;
			hxSize = n;

			LevmarFuncU^ fWrapperU = gcnew LevmarFuncU(this, &Levmar::UnmanagedFuncWrapper);
			GCHandle gch = GCHandle::Alloc(fWrapperU);
			IntPtr unmanagedFp = Marshal::GetFunctionPointerForDelegate(fWrapperU);
			int ret = dlevmar_dif((func)(void*)unmanagedFp, pU, xU, m, n, itmax, optsU, infoU, NULL, NULL, data.ToPointer());

			gch.Free();

			return ret;
		}

		/// <summary>
		/// This function seeks the parameter vector p that best describes the measurements vector x.
		/// More precisely, given a vector function  func : R^m --> R^n with n>=m,
		/// it finds p s.t. func(p) ~= x, i.e. the squared second order (i.e. L2) norm of
		/// e=x-func(p) is minimized.
		///
		/// This function requires an analytic Jacobian. In case the latter is unavailable,
		/// use LevmarDif() .
		///
		/// For more details, see K. Madsen, H.B. Nielsen and O. Tingleff's lecture notes on 
		/// non-linear least squares at http://www.imm.dtu.dk/pubdb/views/edoc_download.php/3215/pdf/imm3215.pdf
		/// </summary>
		/// <param name="f">Delegate to the method expressing the functional relation describing measurements.</param>
		/// <param name="p">Initial parameter estimates. On output has the estimated solution.</param>
		/// <param name="x">Measurement vector. NULL implies a zero vector</param>
		/// <param name="m">Parameter vector dimension.</param>
		/// <param name="n">Measurement vector dimension.</param>
		/// <param name="itmax">Maximum number of iterations</param>
		/// <param name="opts">opts[0-4] = Minimum options. Set to NULL for defaults to be used.</param>
		/// <param name="info">Returns output information regarding the minimization. Set to NULL if don't care.</param>
		/// <returns>The number of iterations (>=0) if successful. LmError (-1) if failed.</returns>
		int LevmarDer(LevmarFunc^f, LevmarJacf^ j, array<double> ^%p, array<double> ^x, int m, int n, int itmax,
			array<double>^ opts, array<double>^% info)
		{
			return LevmarDer(f, j, p, x, m,n, itmax, opts,info, IntPtr::Zero);
		}

		///
		/// <summary>
		/// This function seeks the parameter vector p that best describes the measurements vector x.
		/// More precisely, given a vector function  func : R^m --> R^n with n>=m,
		/// it finds p s.t. func(p) ~= x, i.e. the squared second order (i.e. L2) norm of
		/// e=x-func(p) is minimized.
		///
		/// This function requires an analytic Jacobian. In case the latter is unavailable,
		/// use LevmarDif() .
		///
		/// For more details, see K. Madsen, H.B. Nielsen and O. Tingleff's lecture notes on 
		/// non-linear least squares at http://www.imm.dtu.dk/pubdb/views/edoc_download.php/3215/pdf/imm3215.pdf
		/// </summary>
		/// <param name="f">Delegate to the method expressing the functional relation describing measurements.</param>
		/// <param name="j">Delegate to the method that evaluates the jacobian.</param>
		/// <param name="p">Initial parameter estimates. On output has the estimated solution.</param>
		/// <param name="x">Measurement vector. NULL implies a zero vector</param>
		/// <param name="m">Parameter vector dimension.</param>
		/// <param name="n">Measurement vector dimension.</param>
		/// <param name="itmax">Maximum number of iterations</param>
		/// <param name="opts">opts[0-4] = Minimum options. Set to NULL for defaults to be used.</param>
		/// <param name="info">Returns output information regarding the minimization. Set to NULL if don't care.</param>
		/// <returns>Returns the number of iterations (>=0) if successful. LmError (-1) if failed.</returns>
		int LevmarDer(LevmarFunc^f, LevmarJacf^ j, array<double> ^%p, array<double> ^x, int m, int n, int itmax,
			array<double>^ opts, array<double>^% info, IntPtr data)
		{
			pin_ptr<double> xU = &x[0];
			pin_ptr<double> pU = &p[0];
			pin_ptr<double> optsU = &opts[0];
			pin_ptr<double> infoU = &info[0];

			functionM = f;
			functionJ = j;
			pSize = p->Length;
			hxSize = n;
			jacSize = n * m;

			LevmarFuncU^ fWrapperU = gcnew LevmarFuncU(this, &Levmar::UnmanagedFuncWrapper);
			LevmarJacfU^ jWrapperU = gcnew LevmarJacfU(this, &Levmar::UnmanagedJacfWrapper);
			GCHandle gchFunc = GCHandle::Alloc(fWrapperU);
			GCHandle gchJacf = GCHandle::Alloc(jWrapperU);
			IntPtr unmanagedFp = Marshal::GetFunctionPointerForDelegate(fWrapperU);
			IntPtr unmanagedJp = Marshal::GetFunctionPointerForDelegate(jWrapperU);
			int ret = dlevmar_der((func)(void*)unmanagedFp, (jacf)(void*)unmanagedJp,
				pU, xU, m, n, itmax, optsU, infoU, NULL, NULL, data.ToPointer());

			gchFunc.Free();
			gchJacf.Free();

			return ret;
		}
	};
}
