using LevmarSharp;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LevmarSharpTest
{
	public static class Problems
	{
		private static string[] problemDescription = { "Wood's function", "Osborne's problem" };
		private static int problemIndex;

		private static double[] opts = new double[Levmar.LmOptionsSize];
		private static double[] info = new double[Levmar.LmInfoSize];

		public static void TestWood()
		{
			problemIndex = 0;
			int m = 4; int n = 6;
			double[] p = new[] { -3.0, -1.0, -3.0, -1.0 };
			double[] x = new double[n];

			opts[0] = Levmar.LmInitMu; opts[1] = 1E-15; opts[2] = 1E-15; opts[3] = 1E-20; opts[4] = Levmar.LmDiffDelta;

			Levmar levmar = new Levmar();
			int ret = levmar.LevmarDif(wood, ref p, x, m, n, 1000, opts, ref info);
			PrintOutput(ret, m, p, info);
		}

		public static void TestOsborne()
		{
			problemIndex = 1;
			double[] x33={
				8.44E-1, 9.08E-1, 9.32E-1, 9.36E-1, 9.25E-1, 9.08E-1, 8.81E-1,
				8.5E-1, 8.18E-1, 7.84E-1, 7.51E-1, 7.18E-1, 6.85E-1, 6.58E-1,
				6.28E-1, 6.03E-1, 5.8E-1, 5.58E-1, 5.38E-1, 5.22E-1, 5.06E-1,
				4.9E-1, 4.78E-1, 4.67E-1, 4.57E-1, 4.48E-1, 4.38E-1, 4.31E-1,
				4.24E-1, 4.2E-1, 4.14E-1, 4.11E-1, 4.06E-1};

			int m=5; int n=33;
			double[] p = new[] {0.5, 1.5, -1.0, 1.0E-2, 2.0E-2};
			opts[0] = Levmar.LmInitMu; opts[1] = 1E-15; opts[2] = 1E-15; opts[3] = 1E-20; opts[4] = Levmar.LmDiffDelta;
			Levmar levmar = new Levmar();

			int ret=levmar.LevmarDer(osborne, jacosborne, ref p, x33, m, n, 1000, opts, ref info); // with analytic Jacobian
			//ret=levmar.LevmarDif(osborne, p, x33, m, n, 1000, opts, info, IntPtr.Zero);  // no Jacobian
			PrintOutput(ret, m, p, info);
		}

		private static void PrintOutput(int ret, int m, double[] p, double[] info)
		{
			StringBuilder sb = new StringBuilder();
			sb.AppendLine(string.Format("Results for {0}", problemDescription[problemIndex]));
			sb.AppendLine(string.Format("Levenberg-Marquardt returned {0} in {1:g} iterations, reason: {2:g}", ret, info[5], info[6]));
			sb.AppendLine("Solution:");

			for (int i = 0; i < m; ++i)
				sb.AppendFormat("{0:g7} ", p[i]);
			sb.AppendLine("\n\nMinimization Info:");

			for (int i = 0; i < Levmar.LmInfoSize; i++)
				sb.AppendFormat("{0:g} ", info[i]);
			sb.Append("\n");

			Console.Write(sb.ToString());
		}

		// Wood's function
		private static void wood(double[] p, double[] x, int m, int n, IntPtr data)
		{
			int i;

			for (i = 0; i < n; i += 6)
			{
				x[i] = 10.0 * (p[1] - p[0] * p[0]);
				x[i + 1] = 1.0 - p[0];
				x[i + 2] = Math.Sqrt(90.0) * (p[3] - p[2] * p[2]);
				x[i + 3] = 1.0 - p[2];
				x[i + 4] = Math.Sqrt(10.0) * (p[1] + p[3] - 2.0);
				x[i + 5] = (p[1] - p[3]) / Math.Sqrt(10.0);
			}
		}

		private static void osborne(double[] p, double[] x, int m, int n, IntPtr data)
		{
			int i;
			double t;

			for (i = 0; i < n; ++i)
			{
				t = 10 * i;
				x[i] = p[0] + p[1] * Math.Exp(-p[3] * t) + p[2] * Math.Exp(-p[4] * t);
			}

		}

		private static void jacosborne(double[] p, double[] jac, int m, int n, IntPtr data)
		{
			
			int i, j;
			double t, tmp1, tmp2;

			for(i=j=0; i<n; ++i){
				t=10*i;
				tmp1=Math.Exp(-p[3]*t);
				tmp2=Math.Exp(-p[4]*t);

				jac[j++]=1.0;
				jac[j++]=tmp1;
				jac[j++]=tmp2;
				jac[j++]=-p[1]*t*tmp1;
				jac[j++]=-p[2]*t*tmp2;
			}
		}
	}
}
