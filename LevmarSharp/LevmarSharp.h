// LevmarSharp.h
#include "levmar.h"
#include "malloc.h"
#define NULL 0
#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;

namespace LevmarSharp {
	/// <summary>
	/// A delegate for the method required by the Levenberg-Marquardt algorithm.
	/// </summary>
	/// <param name="p">Initial parameter estimates.</param>
	/// <param name="hx">Measurement vector.</param>
	/// <param name="m">Parameter vector dimension.</param>
	/// <param name="n">Measurement vector dimension.</param>
	/// <param name="data">Additional data, passed uninterpreted to function.</param>
	public delegate void LevmarFunc(array<double>^ p, array<double>^ hx, int m, int n, IntPtr data);

	public ref class Levmar
	{
		[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
		delegate void LevmarFuncU(double *p, double *hx, int m, int n, void *data);
		typedef void (*func)(double *p, double *hx, int m, int n, void *adata);

		int pSize;
		int xSize;
		LevmarFunc^ functionM;

		void UnmanagedFuncWrapper(double *p, double *hx, int m, int n, void *data)
		{
			array<double>^ pM = gcnew array<double>(pSize);
			array<double>^ hxM = gcnew array<double>(xSize);

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

	public:
		static int LmOptionsSize    	= 5; /* max(4, 5) */
		static int LmInfoSize   	 = 10;
		static double LmInitMu    	 =1E-03;
		static double LmDiffDelta    = 1E-06;

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
		/// <returns>The number of iterations used.</returns>
		int LevmarDif(LevmarFunc^ f, array<double> ^p, array<double> ^x, int m, int n, int itmax,
			array<double>^ opts, array<double>^ info, IntPtr data)
		{
			pin_ptr<double> xU = &x[0];
			pin_ptr<double> pU = &p[0];
			pin_ptr<double> optsU = &opts[0];
			pin_ptr<double> infoU = &info[0];

			functionM = f;
			pSize = p->Length;
			xSize = x->Length;

			LevmarFuncU^ fWrapperU = gcnew LevmarFuncU(this, &Levmar::UnmanagedFuncWrapper);
			GCHandle gch = GCHandle::Alloc(fWrapperU);
			IntPtr unmanagedFp = Marshal::GetFunctionPointerForDelegate(fWrapperU);
			int ret = dlevmar_dif((func)(void*)unmanagedFp, pU, xU, m, n, itmax, optsU, infoU, NULL, NULL, data.ToPointer());

			gch.Free();

			return ret;
		}
	};
}
