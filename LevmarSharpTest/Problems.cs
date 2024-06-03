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
        private static string[] problemDescription = { "Wood's function" };
        private static int problemIndex;

        public static void TestWood()
        {
            problemIndex = 0;
            int m = 4; int n = 6;
            double[] p = new[] { -3.0, -1.0, -3.0, -1.0 };
            double[] x = new double[n];
            double[] opts = new double[Levmar.LmOptionsSize];
            double[] info = new double[Levmar.LmInfoSize];
            opts[0] = Levmar.LmInitMu; opts[1] = 1E-15; opts[2] = 1E-15; opts[3] = 1E-20; opts[4] = Levmar.LmDiffDelta;

            Levmar levmar = new Levmar();
            int ret = levmar.LevmarDif(wood, p, x, m, n, 1000, opts, info, IntPtr.Zero);
            PrintOutput(ret, m, p, info);
        }

        private static void PrintOutput(int ret, int m, double[] p, double[] info)
        {
            StringBuilder sb = new StringBuilder();
            sb.AppendLine(string.Format("Results for {0}", problemDescription[problemIndex]));
            sb.AppendLine(string.Format("Levenberg-Marquardt returned {0} in {1:g} iterations, reason: {2:g}", ret, info[5], info[6]));
            sb.AppendLine("Solution:");

            //printf("Results for %s:\n", probname[problem]);
            //printf("Levenberg-Marquardt returned %d in %g iter, reason %g\nSolution: ", ret, info[5], info[6]);
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
    }
}