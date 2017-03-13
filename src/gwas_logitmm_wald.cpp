#include <Rcpp.h>
#include "ai-reml-logit-1k-covar.h"
#include "matrix4.h"
#include <ctime>
#include <cmath>
#define BLOCK 20 

//[[Rcpp::export]]
List GWAS_logitmm_wald(XPtr<matrix4> pA, NumericVector mu, NumericVector Y, NumericMatrix X, 
                       NumericMatrix K, int beg, int end, double tol) {
  Map_MatrixXd y(as<Map<MatrixXd> >(Y));
  Map_MatrixXd x(as<Map<MatrixXd> >(X));
  Map_MatrixXd kk(as<Map<MatrixXd> >(K));

  int n = y.rows();
  int r = x.cols();

  // declare vectors containing result
  VectorXd TAU(end-beg+1);
  VectorXd BETA(end-beg+1);
  VectorXd SDBETA(end-beg+1);

  // initial values for beta, tau
  double tau; 
  int niter;
  MatrixXd P(n,n);
  VectorXd omega(n);
  VectorXd beta(r);
  MatrixXd varbeta(r,r);

  for(int i = 0; i < n; i++) x(i,r-1) = 0;
  AIREML1_logit(y, x, kk, true, 1e-6, 100, tol, false, tau, niter, P, omega, beta, varbeta, false, false);

  //clock_t chaviro(0);
  // clock_t begin_t = clock();

  // Rcout << min_h2 << " < h2 < " << max_h2 << "\n";
  for(int i = beg; i <= end; i++) {
    if( std::isnan(mu(i)) || mu(i) == 0 || mu(i) == 2 ) {
      TAU(i-beg) = NAN;
      BETA(i-beg) = NAN;
      SDBETA(i-beg) = NAN;
      continue;
    }
    // remplir dernière colonne de x par génotype au SNP (manquant -> mu)
    for(int ii = 0; ii < pA->true_ncol-1; ii++) {
      uint8_t xx = pA->data[i][ii];
      for(int ss = 0; ss < 4; ss++) {
        x(4*ii+ss, r-1) = ((xx&3) != 3)?(xx&3):mu(i);
        xx >>= 2;
      }
    }
    { int ii = pA->true_ncol-1;
      uint8_t xx = pA->data[i][ii];
      for(int ss = 0; ss < 4 && 4*ii+ss < pA->ncol; ss++) {
        x(4*ii+ss, r-1) = ((xx&3) != 3)?(xx&3):mu(i);
        xx >>= 2;
      }
    }

    // use last computed values as starting point...
    AIREML1_logit(y, x, kk, true, 1e-6, 100, tol, false, tau, niter, P, omega, beta, varbeta, true, true);

    TAU(i-beg) = tau;
    BETA(i-beg) = beta(r-1);
    SDBETA(i-beg) = sqrt(varbeta(r-1,r-1));

  }

  //cout << (float) chaviro / CLOCKS_PER_SEC << "\n";
  List R;
  R["tau"] = TAU;
  R["beta"] = BETA;
  R["sd"] = SDBETA;
  return R;
}

//[[Rcpp::export]]
List GWAS_logitmm_wald_f(XPtr<matrix4> pA, NumericVector mu, NumericVector Y, NumericMatrix X, 
                        NumericMatrix K, int beg, int end, double tol) {
  int n = Y.size();
  int r = X.ncol();

  // recopiage des matrices... en float
  MatrixXf y(n,1);
  MatrixXf x(n,r);
  MatrixXf kk(n,n);
  for(int i = 0; i < n; i++) y(i,0) = (float) Y[i];

  for(int i = 0; i < n; i++) 
    for(int j = 0; j < r; j++)
      x(i,j) = (float) X(i,j);

  for(int i = 0; i < n; i++) 
    for(int j = 0; j < n; j++)
      kk(i,j) = (float) K(i,j);



  // declare vectors containing result
  VectorXf TAU(end-beg+1);
  VectorXf BETA(end-beg+1);
  VectorXf SDBETA(end-beg+1);

  // initial values for beta, tau
  float tau; 
  int niter;
  MatrixXf P(n,n);
  VectorXf omega(n);
  VectorXf beta(r);
  MatrixXf varbeta(r,r);
  
  for(int i = 0; i < n; i++) x(i,r-1) = 0;
  AIREML1_logit_f(y, x, kk, true, 1e-6, 100, tol, false, tau, niter, P, omega, beta, varbeta, false, false);

  //clock_t chaviro(0);
  // clock_t begin_t = clock();

  // Rcout << min_h2 << " < h2 < " << max_h2 << "\n";
  for(int i = beg; i <= end; i++) {
    // remplir dernière colonne de x par génotype au SNP (manquant -> mu)
    for(int ii = 0; ii < pA->true_ncol-1; ii++) {
      uint8_t xx = pA->data[i][ii];
      for(int ss = 0; ss < 4; ss++) {
        x(4*ii+ss, r-1) = ((xx&3) != 3)?(xx&3):mu(i);
        xx >>= 2;
      }
    }
    { int ii = pA->true_ncol-1;
      uint8_t xx = pA->data[i][ii];
      for(int ss = 0; ss < 4 && 4*ii+ss < pA->ncol; ss++) {
        x(4*ii+ss, r-1) = ((xx&3) != 3)?(xx&3):mu(i);
        xx >>= 2;
      }
    }

    // use last computed values as starting point...
    AIREML1_logit_f(y, x, kk, true, 1e-6, 100, tol, false, tau, niter, P, omega, beta, varbeta, true, true);

    TAU(i-beg) = tau;
    BETA(i-beg) = beta(r-1);
    SDBETA(i-beg) = sqrt(varbeta(r-1,r-1));

  }

  //cout << (float) chaviro / CLOCKS_PER_SEC << "\n";
  List R;
  R["tau"] = TAU;
  R["beta"] = BETA;
  R["sd"] = SDBETA;
  return R;
}


RcppExport SEXP gg_GWAS_logitmm_wald(SEXP pASEXP, SEXP muSEXP, SEXP YSEXP, SEXP XSEXP, SEXP KSEXP, SEXP begSEXP, SEXP endSEXP, SEXP tolSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XPtr<matrix4> >::type pA(pASEXP);
    Rcpp::traits::input_parameter< NumericVector >::type mu(muSEXP);
    Rcpp::traits::input_parameter< NumericVector >::type Y(YSEXP);
    Rcpp::traits::input_parameter< NumericMatrix >::type X(XSEXP);
    Rcpp::traits::input_parameter< NumericMatrix >::type K(KSEXP);
    Rcpp::traits::input_parameter< int >::type beg(begSEXP);
    Rcpp::traits::input_parameter< int >::type end(endSEXP);
    Rcpp::traits::input_parameter< double >::type tol(tolSEXP);
    rcpp_result_gen = Rcpp::wrap(GWAS_logitmm_wald(pA, mu, Y, X, K, beg, end, tol));
    return rcpp_result_gen;
END_RCPP
}

RcppExport SEXP gg_GWAS_logitmm_wald_f(SEXP pASEXP, SEXP muSEXP, SEXP YSEXP, SEXP XSEXP, SEXP KSEXP, SEXP begSEXP, SEXP endSEXP, SEXP tolSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XPtr<matrix4> >::type pA(pASEXP);
    Rcpp::traits::input_parameter< NumericVector >::type mu(muSEXP);
    Rcpp::traits::input_parameter< NumericVector >::type Y(YSEXP);
    Rcpp::traits::input_parameter< NumericMatrix >::type X(XSEXP);
    Rcpp::traits::input_parameter< NumericMatrix >::type K(KSEXP);
    Rcpp::traits::input_parameter< int >::type beg(begSEXP);
    Rcpp::traits::input_parameter< int >::type end(endSEXP);
    Rcpp::traits::input_parameter< double >::type tol(tolSEXP);
    rcpp_result_gen = Rcpp::wrap(GWAS_logitmm_wald_f(pA, mu, Y, X, K, beg, end, tol));
    return rcpp_result_gen;
END_RCPP
}

