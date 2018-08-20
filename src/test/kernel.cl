/*__kernel void square(
//   __global float* input,
//   __global float* output,
//   const unsigned int count)
//{
//   int i = get_global_id(0);
//   if(i < count)
//       output[i] = input[i] * input[i];
//}*/

__kernel void gemv1(__global float * a,
                    __global float * x,
                    __global float * y,
                    const unsigned int m,
                    const unsigned int n)
{
  float sum = 0.0f;
  int i = get_global_id(0); // row index
  for (int k=0; k<n; k++)
    {
      sum += a[i + m * k] * x[k];
    }
  y[i] = sum;
}
