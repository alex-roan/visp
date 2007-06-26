/****************************************************************************
 *
 * $Id: vpMatrix.cpp,v 1.34 2007-06-26 09:23:22 asaunier Exp $
 *
 * Copyright (C) 1998-2006 Inria. All rights reserved.
 *
 * This software was developed at:
 * IRISA/INRIA Rennes
 * Projet Lagadic
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * http://www.irisa.fr/lagadic
 *
 * This file is part of the ViSP toolkit
 *
 * This file may be distributed under the terms of the Q Public License
 * as defined by Trolltech AS of Norway and appearing in the file
 * LICENSE included in the packaging of this file.
 *
 * Licensees holding valid ViSP Professional Edition licenses may
 * use this file in accordance with the ViSP Commercial License
 * Agreement provided with the Software.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Contact visp@irisa.fr if any conditions of this licensing are
 * not clear to you.
 *
 * Description:
 * Matrix manipulation.
 *
 * Authors:
 * Eric Marchand
 *
 *****************************************************************************/



/*!
  \file vpMatrix.cpp
  \brief Definition of the vpMatrix class
*/

/*
  \class vpMatrix

  \brief Provide simple Matrices computation

  \author Eric Marchand   (Eric.Marchand@irisa.fr) Irisa / Inria Rennes
*/


#include <stdlib.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <assert.h>

#include <visp/vpMath.h>
#include <visp/vpMatrix.h>
#include <visp/vpTranslationVector.h>


// Exception
#include <visp/vpException.h>
#include <visp/vpMatrixException.h>

// Debug trace
#include <visp/vpDebug.h>

#define DEBUG_LEVEL1 0

/*!
  \brief initialization of the object matrix.
  Number of columns and rows are zero.
*/

void
vpMatrix::init()
{

  rowNum = 0  ;
  colNum = 0 ;

  data = NULL ;
  rowPtrs = NULL ;

  dsize = 0 ;
  trsize =0 ;
}

/*!
  \brief basic constructor of the matrix class
  Construction of the object matrix.
  Number of columns and rows are zero.
*/
vpMatrix::vpMatrix()
{
  init() ;
}


/*!
  Constructor.

  Initialize a matrix with 0.

  \param r : Matrix number of rows.
  \param c : Matrix number of columns.
*/
vpMatrix::vpMatrix(int r,int c)
{
  init() ;
  resize(r, c);
}

/*!
  \brief submatrix constructor
*/
vpMatrix::vpMatrix(const vpMatrix &m,
		   int r,
		   int c, int nrows, int ncols)
{
  init() ;

  if ( (r<0) || (c<0) )
  {
    vpERROR_TRACE("\n\t\t Illegal subMatrix operation") ;
    throw(vpMatrixException(vpMatrixException::subMatrixError,
			    "\n\t\t Illegal subMatrix operation")) ;
  }

  if (((r + nrows) > m.rowNum) || ((c + ncols) > m.colNum))
  {
    vpERROR_TRACE("\n\t\t SubvpMatrix larger than vpMatrix") ;
    throw(vpMatrixException(vpMatrixException::subMatrixError,
			    "\n\t\t SubvpMatrix larger than vpMatrix")) ;
  }

  init(m,r,c,nrows,ncols);
}

//! copie constructor
vpMatrix::vpMatrix(const vpMatrix& m)
{
  init() ;

  resize(m.rowNum,m.colNum);

  memcpy(data,m.data,rowNum*colNum*sizeof(double)) ;

  // MODIF EM 13/6/03
  /*for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
    rowPtrs[i][j] = m.rowPtrs[i][j];
    }
    }
  */
}


/*!
  \brief set the size of the matrix, initialization with a zero matrix

  \param nrows : number of rows
  \param ncols : number of column
  \param flagNullify : if true, then the matrix is re-initialized to 0
  afet resize. If false, the initial values from the common part of the
  matrix (comon part between old and new version of the matrix) are kept.
  Default value is true.

  \return OK or MEMORY_FAULT if memory cannot be allocated
*/

void
vpMatrix::resize(const int nrows, const int ncols, const bool flagNullify)
{

  if ((nrows == rowNum) && (ncols == colNum))
  {
    if (flagNullify)
      { memset(this->data,0,this->dsize*sizeof(double)) ;}
  }
  else
  {
    const bool recopyNeeded = (ncols != this ->colNum);
    double * copyTmp = NULL;
    int rowTmp = 0, colTmp=0;

    vpDEBUG_TRACE (25, "Recopy case per case is required iff number of "
		 "cols has changed (structure of double array is not "
		 "the same in this case.");
    if (recopyNeeded)
      {
	copyTmp = new double[this->dsize];
	memcpy (copyTmp, this ->data, sizeof(double)*this->dsize);
	rowTmp=this->rowNum; colTmp=this->colNum;
      }

    vpDEBUG_TRACE (25, "Reallocation of this->data array.");
    this->dsize = nrows*ncols;
    this->data = (double*)realloc(this->data, this->dsize*sizeof(double));
    if ((NULL == this->data) && (0 != this->dsize))
    {
      vpERROR_TRACE("\n\t\tMemory allocation error when allocating data") ;
      throw(vpException(vpException::memoryAllocationError,
			"\n\t\t Memory allocation error when "
			"allocating data")) ;
    }

    vpDEBUG_TRACE (25, "Reallocation of this->trsize array.");
    this->trsize = nrows;
    this->rowPtrs = (double**)realloc (this->rowPtrs, this->trsize*sizeof(double*));
    if ((NULL == this->rowPtrs) && (0 != this->dsize))
    {
      vpERROR_TRACE("\n\t\tMemory allocation error when allocating rowPtrs") ;
      throw(vpException(vpException::memoryAllocationError,
			"\n\t\t Memory allocation error when "
			"allocating rowPtrs")) ;
    }

    vpDEBUG_TRACE (25, "Recomputation this->trsize array values.");
    {
      double **t= rowPtrs;
      for (int i=0; i<dsize; i+=ncols)  { *t++ = this->data + i; }
    }

    this->rowNum = nrows; this->colNum = ncols;

    vpDEBUG_TRACE (25, "Recopy of this->data array values or nullify.");
    if (flagNullify)
      { memset(this->data,0,this->dsize*sizeof(double)) ;}
    else
      {
	if (recopyNeeded)
	  {
	    vpDEBUG_TRACE (25, "Recopy...");
	    const int minRow = (this->rowNum<rowTmp)?this->rowNum:rowTmp;
	    const int minCol = (this->colNum<colTmp)?this->colNum:colTmp;
	    for (int i=0; i<this->rowNum; ++i)
	      for (int j=0; j<this->colNum; ++j)
		{
		  if ((minRow > i) && (minCol > j))
		    {
		      (*this)[i][j] = copyTmp [i*colTmp+j];
		      vpCDEBUG (25) << i << "x" << j << "<- " << i*colTmp+j
				  << "=" << copyTmp [i*colTmp+j] << std::endl;
		    }
		  else {(*this)[i][j] = 0;}
		}
	  }
	else { vpDEBUG_TRACE (25,"Nothing to do: already done by realloc.");}
      }

    if (copyTmp != NULL) delete [] copyTmp;
  }

}


void
vpMatrix::init(const vpMatrix &m,int r, int c, int nrows, int ncols)
{
  try {
    resize(nrows, ncols) ;
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    std::cout << me << std::endl ;
    throw ;
  }

  int rnrows = r+nrows ;
  int cncols = c+ncols ;
  for (int i=r ; i < rnrows; i++)
    for (int j=c ; j < cncols; j++)
      (*this)[i-r][j-c] = m[i][j] ;
}

/*!
  \brief Destruction of the matrix  (Memory de-allocation)
*/
void
vpMatrix::kill()
{
  if (data != NULL )
  {
    free(data);
    data=NULL;
  }

  if (rowPtrs!=NULL)
  {
    free(rowPtrs);
    rowPtrs=NULL ;
  }
}
/*!
  \brief Destructor (Memory de-allocation)
*/
vpMatrix::~vpMatrix()
{
  kill() ;
}


/*!
  \brief Copy operator.
  Allow operation such as A = B

  \param B : matrix to be copied.
*/
vpMatrix &
vpMatrix::operator=(const vpMatrix &B)
{
  try {
    resize(B.rowNum, B.colNum) ;
    // suppress by em 5/12/06
    //    *this = 0;
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    std::cout << me << std::endl ;
    throw ;
  }

  memcpy(data,B.data,dsize*sizeof(double)) ;

  return *this;
}

//! set all the element of the matrix A to x
vpMatrix &
vpMatrix::operator=(double x)
{
  for (int i=0; i<dsize; i++)
  {
    *(data+i) = x;
  }
  return *this;
}

/*!
  \brief Assigment from an array of double
*/
vpMatrix &
vpMatrix::operator<<( double *x )
{

  for (int i=0; i<rowNum; i++) {
    for (int j=0; j<colNum; j++) {
      rowPtrs[i][j] = *x++;
    }
  }
  return *this;
}


//---------------------------------
// Matrix operations.
//---------------------------------

//! operation C = A * B (A is unchanged)
vpMatrix
vpMatrix::operator*(const vpMatrix &B) const
{
  vpMatrix p ;


  try {
    p.resize(rowNum,B.colNum) ;
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    std::cout << me << std::endl ;
    throw ;
  }

  if (colNum != B.rowNum)
  {
    vpERROR_TRACE("\n\t\tvpMatrix mismatch in vpMatrix/vpMatrix multiply") ;
    throw(vpMatrixException(vpMatrixException::incorrectMatrixSizeError,
			    "\n\t\tvpMatrix mismatch in "
			    "vpMatrix/vpMatrix multiply")) ;
  }

  // 5/12/06 some "very" simple optimization to avoid indexation
  int BcolNum = B.colNum ;
  int BrowNum = B.rowNum ;
  int i,j,k ;
  double **BrowPtrs = B.rowPtrs;
  for (i=0;i<rowNum;i++)
  {
    double *rowptri = rowPtrs[i] ;
    double *pi = p[i] ;
    for (j=0;j<BcolNum;j++)
	  {
	    double s =0 ;
	    for (k=0;k<BrowNum;k++)
	      s +=rowptri[k] * BrowPtrs[k][j];
	    pi[j] = s ;
	  }
  }
  return p;
}

//! operation C = A + B (A is unchanged)
vpMatrix
vpMatrix::operator+(const vpMatrix &B) const
{
  vpMatrix v ;


  try {
    v.resize(rowNum,colNum) ;
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    std::cout << me << std::endl ;
    throw ;
  }

  if ( (colNum != B.getCols())||(rowNum != B.getRows()))
  {
    vpERROR_TRACE("\n\t\t vpMatrix mismatch in vpMatrix/vpMatrix addition") ;
    throw(vpMatrixException(vpMatrixException::incorrectMatrixSizeError,
			    "\n\t\t vpMatrix mismatch in "
			    "vpMatrix/vpMatrix addition")) ;

  }
  int i;
  // MODIF EM 16/6/03
  /*int j;
    for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)
    {
    v.rowPtrs[i][j] = B.rowPtrs[i][j]+rowPtrs[i][j];
    }
  */
  for (i=0;i<dsize;i++)
  {
    *(v.data + i) = *(B.data + i) + *(data + i) ;
  }
  return v;
}

//! operation C = A - B (A is unchanged)
vpMatrix
vpMatrix::operator-(const vpMatrix &B) const
{
  vpMatrix v ;
  try {
    v.resize(rowNum,colNum) ;
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    std::cout << me << std::endl ;
    throw ;
  }

  if ( (colNum != B.getCols())||(rowNum != B.getRows()))
  {
    vpERROR_TRACE("\n\t\t vpMatrix mismatch in vpMatrix/vpMatrix substraction") ;
    throw(vpMatrixException(vpMatrixException::incorrectMatrixSizeError,
			    "\n\t\t vpMatrix mismatch in "
			    "vpMatrix/vpMatrix substraction")) ;

  }

  int i;

  // MODIF EM 16/6/03
  for (i=0;i<dsize;i++)
  {
    *(v.data + i) = *(data + i) - *(B.data + i) ;
  }
  /*
    int j;
    for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)
    v.rowPtrs[i][j] = rowPtrs[i][j]-B.rowPtrs[i][j];
  */
  return v;
}

//! operation A = A + B

vpMatrix &vpMatrix::operator+=(const vpMatrix &B)
{
  if ( (colNum != B.getCols())||(rowNum != B.getRows()))
  {
    vpERROR_TRACE("\n\t\t vpMatrix mismatch in vpMatrix +=  addition") ;
    throw(vpMatrixException(vpMatrixException::incorrectMatrixSizeError,
			    "\n\t\t vpMatrix mismatch in "
			    "vpMatrix += addition")) ;

  }




  int i;

  // MODIF EM 16/6/03
  for (i=0;i<dsize;i++)
  {
    *(data + i) += *(B.data + i) ;
  }
  /* int j;
     for (i=0;i<rowNum;i++)
     for(j=0;j<colNum;j++)
     rowPtrs[i][j] += B.rowPtrs[i][j];
  */
  return *this;
}

//! operation A = A - B

vpMatrix & vpMatrix::operator-=(const vpMatrix &B)
{
  if ( (colNum != B.getCols())||(rowNum != B.getRows()))
  {
    vpERROR_TRACE("\n\t\t vpMatrix mismatch in vpMatrix -= substraction") ;
    throw(vpMatrixException(vpMatrixException::incorrectMatrixSizeError,
			    "\n\t\t vpMatrix mismatch in "
			    "vpMatrix -= substraction")) ;

  }




  int i;
  // MODIF EM 16/6/03
  for (i=0;i<dsize;i++)
  {
    *(data + i) -= *(B.data + i) ;
  }
  /*
    int j; for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)
    rowPtrs[i][j] -= B.rowPtrs[i][j];
  */
  return *this;
}

//! C = -A  (A is unchanged)

vpMatrix vpMatrix::operator-() const //negate
{
  vpMatrix C ;



  try {
    C.resize(rowNum, colNum) ;
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    std::cout << me << std::endl ;
    throw ;
  }


  for (int i=0;i<dsize;i++)
  {
    *(C.data + i) = -*(data + i) ;
  }
  /*
    for (int i=0; i<rowNum; i++)
    for (int j=0; j<colNum; j++)
    C[i][j] = - rowPtrs[i][j];
  */
  return C;
}

//!return sum of the Aij^2 (for all i, for all j)
double
vpMatrix::sumSquare() const
{
  double sum=0.0;
  double x ;


  double *d = data ;
  double *n = data+dsize ;
  while (d < n )
  {
    x = *d++ ;
    sum += x*x ;
  }
/*
  for (int i=0;i<dsize;i++)
  {
    x = *(data + i) ;
    sum += x*x ;
  }

    for (int i=0; i<rowNum; i++)
    for (int j=0; j<colNum; j++)
    sum += rowPtrs[i][j]*rowPtrs[i][j];
  */

  return sum;
}

//---------------------------------
// Matrix/vector operations.
//---------------------------------

//! operation c = A * b (A is unchanged, c and b are vectors)
vpColVector
vpMatrix::operator*(const vpColVector &b) const
{

  vpColVector c(rowNum);


  if (colNum != b.getRows())
  {
    vpERROR_TRACE("vpMatrix mismatch in vpMatrix/vector multiply") ;
    throw(vpMatrixException::incorrectMatrixSizeError) ;
  }



  c = 0.0;

  for (int j=0;j<colNum;j++) {
    {
      double bj = b[j] ; // optimization em 5/12/2006
      for (int i=0;i<rowNum;i++) {
	c[i]+=rowPtrs[i][j] * bj;
      }
    }
  }

  return c ;
}

//! operation c = A * b (A is unchanged, c and b are translation vectors)
vpTranslationVector
vpMatrix::operator*(const vpTranslationVector &b) const
{
  vpTranslationVector c;

  for (int j=0;j<3;j++) c[j]=0 ;

  for (int j=0;j<3;j++) {
    {
      double bj = b[j] ; // optimization em 5/12/2006
      for (int i=0;i<3;i++) {
	c[i]+=rowPtrs[i][j] * bj;
      }
    }
  }
  return c ;
}

//---------------------------------
// Matrix/real operations.
//---------------------------------

/*!
  \relates vpMatrix
  \brief multiplication by a scalar  Cij = x*Bij
*/
vpMatrix operator*(const double &x,const vpMatrix &B)
{
  // Modif EM 13/6/03
  vpMatrix v ;

  try {
    v.resize(B.getRows(),B.getCols());
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    std::cout << me << std::endl ;
    throw ;
  }

  int Brow = B.getRows() ;
  int Bcol = B.getCols() ;
  for (int i=0;i<Brow; i++)
    {
      double *vi = v[i] ;
      double *Bi = B[i] ;
      for (int j=0 ; j < Bcol;j++)
	vi[j] = Bi[j]*x;
    }
  return v ;
}

//! Cij = Aij * x (A is unchanged)
vpMatrix vpMatrix::operator*(double x) const
{
  vpMatrix v;

  try {
    v.resize(rowNum,colNum);
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    std::cout << me << std::endl ;
    throw ;
  }



  for (int i=0;i<dsize;i++)
    *(v.data+i) = *(data+i)*x;

  // Modif EM 13/6/03
  /*
    int i;
    int j;
    for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)  v.rowPtrs[i][j] = rowPtrs[i][j]*x;
  */
  return v;
}

//! Cij = Aij / x (A is unchanged)
vpMatrix  vpMatrix::operator/(double x) const
{
  vpMatrix v;

  try {
    v.resize(rowNum,colNum);
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    vpCERROR << me << std::endl ;
    throw ;
  }


  double  xinv = 1/x ;

  for (int i=0;i<dsize;i++)
    *(v.data+i) = *(data+i)*xinv ;

  return v;
}


//! Add x to all the element of the matrix : Aij = Aij + x
vpMatrix & vpMatrix::operator+=(double x)
{


  for (int i=0;i<dsize;i++)
    *(data+i) += x;

  return *this;
}


//! Substract x to all the element of the matrix : Aij = Aij - x
vpMatrix & vpMatrix::operator-=(double x)
{


  for (int i=0;i<dsize;i++)
    *(data+i) -= x;
  /*
    int i;int j;
    for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)
    rowPtrs[i][j] -= x;
  */
  return *this;
}

//! Multiply  all the element of the matrix by x : Aij = Aij * x
vpMatrix & vpMatrix::operator*=(double x)
{


  for (int i=0;i<dsize;i++)
    *(data+i) *= x;
  /*
    int i;int j;
    for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)  rowPtrs[i][j] *= x;
  */
  return *this;
}

//! Divide  all the element of the matrix by x : Aij = Aij / x
vpMatrix & vpMatrix::operator/=(double x)
{

  double xinv = 1/x ;
  for (int i=0;i<dsize;i++)
    *(data+i) *= xinv;
  /*
    int i;int j;
    for (i=0;i<rowNum;i++)
    for(j=0;j<colNum;j++)  rowPtrs[i][j] /= x;
  */
  return *this;
}


//----------------------------------------------------------------
// Matrix Operation
//----------------------------------------------------------------

/*!
  \brief set the matrix to identity
*/
void
vpMatrix::setIdentity()
{

  if (rowNum != colNum)
  {
    vpERROR_TRACE("non square matrix") ;
    throw(vpMatrixException(vpMatrixException::matrixError)) ;
  }



  int i,j;
  for (i=0;i<rowNum;i++)
    for (j=0;j<colNum;j++)
      if (i==j) (*this)[i][j] = 1 ; else (*this)[i][j] = 0;
}

/*!
  \brief set the matrix to identity

  eye(n) is an n-by-n matrix with ones on the diagonal and zeros
  elsewhere

  \sa eye(n) is also a matlab function
*/
void
vpMatrix::eye(int n)
{
  try {
    eye(n,n) ;
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    vpCERROR << me << std::endl ;
    throw ;
  }
}
/*!
  \brief eye(m,n) is an m-by-n matrix with ones on the diagonal and zeros
  elsewhere

  \sa eye(m,n) is also a matlab function
*/
void
vpMatrix::eye(int m, int n)
{
  try {
     resize(m,n) ;
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    vpCERROR << me << std::endl ;
    throw ;
  }


  for (int i=0; i<rowNum; i++)
    for (int j=0; j<colNum; j++)
      if (i == j) (*this)[i][j] = 1;
      else        (*this)[i][j] = 0;

}


/*!
  \brief Transpose the matrix C = A^T
  \return  A^T
*/
vpMatrix vpMatrix::t() const
{
  vpMatrix At ;

  try {
    At.resize(colNum,rowNum);
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    vpCERROR << me << std::endl ;
    throw ;
  }



  int i,j;
  for (i=0;i<rowNum;i++)
  {
    double *coli = (*this)[i] ;
    for (j=0;j<colNum;j++)
      At[j][i] = coli[j];
  }
  return At;
}


/*!
  \brief Compute the AtA operation B = A^T*A
  \return  A^T*A
*/
vpMatrix vpMatrix::AtA() const
{
  vpMatrix AtA ;
  try {
    AtA.resize(colNum,colNum);
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    vpCERROR << me << std::endl ;
    throw ;
  }

  int i,j,k;
  double s;
  double *ptr;
  double *AtAi;
  for (i=0;i<colNum;i++)
  {
    AtAi = AtA[i] ;
    for (j=0;j<i;j++)
    {
      ptr=data;
      s = 0 ;
      for (k=0;k<rowNum;k++)
      {
        s +=(*(ptr+i)) * (*(ptr+j));
        ptr+=colNum;
      } 
      *AtAi++ = s ;
      AtA[j][i] = s;
    }
    ptr=data;
    s = 0 ;
    for (k=0;k<rowNum;k++)
    {
      s +=(*(ptr+i)) * (*(ptr+i));
      ptr+=colNum;
    }
    *AtAi = s ;
  }
  
  return AtA;
}

/*!
  \brief solve a linear system AX = B using an SVD decomposition

  non destructive wrt. A and B

  \sa this function SVDcmp and SVDksb for solving the system
*/
void
vpMatrix::solveBySVD(const vpColVector& b, vpColVector& x) const
{
  x = pseudoInverse(1e-6)*b ;
}


/*!
  \brief solve a linear system AX = B using an SVD decomposition

  non destructive wrt. A and B

  \sa SVDcmp and SVDksb
*/
vpColVector vpMatrix::SVDsolve(const vpColVector& B) const
{
  vpColVector X(colNum);

  solveBySVD(B, X);
  return X;
}



void
vpMatrix::svd(vpColVector& w, vpMatrix& v)
{
  if (DEBUG_LEVEL1 == 0) /* no verification */
  {
    w.resize( this->getCols() );
    v.resize( this->getCols(), this->getCols() );
#ifdef VISP_HAVE_GSL  /* be careful of the copy below */
    svdGsl(w,v) ;
#else
    svdNr(w,v) ;
#endif
  //svdNr(w,v) ;
  }
  else  /* verification of the SVD */
  {
    int pb = 0;
    int i,j,k,nrows,ncols;
    vpMatrix A, Asvd;

    A = (*this);        /* copy because svd is destructive */

    w.resize( this->getCols() );
    v.resize( this->getCols(), this->getCols() );
#ifdef VISP_HAVE_GSL  /* be careful of the copy above */
    svdGsl(w,v) ;
#else
    svdNr(w,v) ;
#endif
  //svdNr(w,v) ;

    nrows = A.getRows();
    ncols = A.getCols();
    Asvd.resize(nrows,ncols);

    for (i = 0 ; i < nrows ; i++)
    {
      for (j = 0 ; j < ncols ; j++)
      {
        Asvd[i][j] = 0.0;
        for (k=0 ; k < ncols ; k++) Asvd[i][j] += (*this)[i][k]*w[k]*v[j][k];
      }
    }
    for (i=0;i<nrows;i++)
    {
      for (j=0;j<ncols;j++) if (fabs(A[i][j]-Asvd[i][j]) > 1e-6) pb = 1;
    }
    if (pb == 1)
    {
      printf("pb in SVD\n");
      std::cout << " A : " << std::endl << A << std::endl;
      std::cout << " Asvd : " << std::endl << Asvd << std::endl;
    }
    //    else printf("SVD ok ;-)\n");  /* It's so good... */
  }
}
/*!
  \brief Compute the pseudo inverse of the matrix Ap = A^+
  \param Ap = A^+ the pseudo inverse
  \param th threshold used to test the singular values
  \return Return the rank of the matrix A
*/

int
vpMatrix::pseudoInverse(vpMatrix &Ap, double th) const
{
  vpColVector sv ;
  return   pseudoInverse(Ap,sv,th) ;
}

/*!
  \brief Compute and return the pseudo inverse of the matrix : \f$ A^+ \f$
  \param svThreshold : Threshold used to test the singular values.

  \return Pseudo inverse of the matrix
*/
vpMatrix
vpMatrix::pseudoInverse(double svThreshold) const
{
  vpMatrix Ap ;
  vpColVector sv ;
  pseudoInverse(Ap, sv, svThreshold) ;
  return   Ap ;
}

/*!
  \brief Compute the pseudo inverse of the matrix Ap = A^+
  \param Ap = A^+ the pseudo inverse
  \param sv singular values
  \param th threshold used to test the singular values
  \return Return the rank of the matrix A
*/
int
vpMatrix::pseudoInverse(vpMatrix &Ap, vpColVector &sv, double seuilvp) const
{
  vpMatrix imA, imAt ;
  return pseudoInverse(Ap,sv,seuilvp, imA, imAt) ;
}

/*!
  \brief Compute the pseudo inverse of the matrix Ap = A^+ along with Ker A, Ker A^T, Im A and Im A^T

  Pseudo Inverse, Kernel and Image are computed using the SVD decomposition

  A is an m x n matrix,
  if m >=n the svd works on A other wise it works on A^T

  Therefore if m>=n we have

  \f[
  {\bf A}_{m\times n} = {\bf U}_{m\times m} {\bf S}_{m\times n} {\bf V^\top}_{n\times n}
\f]
  \f[
  {\bf A}_{m\times n} = \left[\begin{array}{ccc}\mbox{Im} {\bf A} & | &
  \mbox{Ker} {\bf A^\top} \end{array} \right] {\bf S}
  \left[
  \begin{array}{c} (\mbox{Im} {\bf A^\top})^\top \\   (\mbox{Ker}{\bf A})^\top \end{array}\right]
  \f]
  where
  Im(A) is an m x r matrix (r is the rank of A) and
  Im(A^T) is an r x n matrix



  \param Ap = A^+ the pseudo inverse
  \param sv singular values
  \param th threshold used to test the singular values
  \param ImAt : Image A^T
  \param ImA: Image  A
  \return Return the rank of the matrix A

*/
int
vpMatrix::pseudoInverse(vpMatrix &Ap,
			vpColVector &sv, double seuilvp,
			vpMatrix &imA,
			vpMatrix &imAt) const
{

  int i, j, k ;

  int nrows, ncols;
  int nrows_orig = getRows() ;
  int ncols_orig = getCols() ;
  Ap.resize(ncols_orig,nrows_orig) ;

  if (nrows_orig >=  ncols_orig)
  {
    nrows = nrows_orig;
    ncols = ncols_orig;
  }
  else
  {
    nrows = ncols_orig;
    ncols = nrows_orig;
  }

  vpMatrix a(nrows,ncols) ;
  vpMatrix a1(ncols,nrows);
  vpMatrix v(ncols,ncols) ;
  sv.resize(ncols) ;

  if (nrows_orig >=  ncols_orig) a = *this;
  else a = (*this).t();

  a.svd(sv,v);

  // compute the highest singular value and the rank of h
  double maxsv = 0 ;
  for (i=0 ; i < ncols ; i++)
     if (fabs(sv[i]) > maxsv) maxsv = fabs(sv[i]) ;

  int rank = 0 ;
  for (i=0 ; i < ncols ; i++)
    if (fabs(sv[i]) > maxsv*seuilvp) rank++ ;



  /*------------------------------------------------------- */
  for (i = 0 ; i < ncols ; i++)
  {
    for (j = 0 ; j < nrows ; j++)
    {
      a1[i][j] = 0.0;

      for (k=0 ; k < ncols ; k++)
    	if (fabs(sv[k]) > maxsv*seuilvp)
  	{
	    a1[i][j] += v[i][k]*a[j][k]/sv[k];
        }
    }
  }
  if (nrows_orig >=  ncols_orig) Ap = a1;
  else Ap = a1.t();

  if (nrows_orig >=  ncols_orig)
  {
    //  compute dim At
    imAt.resize(ncols_orig,rank) ;
    for (i=0 ; i  < ncols_orig ; i++)
      for (j=0 ; j < rank ; j++)
	imAt[i][j] = v[i][j] ;

    //  compute dim A
    imA.resize(nrows_orig,rank) ;
    for (i=0 ; i  < nrows_orig ; i++)
      for (j=0 ; j < rank ; j++)
	imA[i][j] = a[i][j] ;
  }
  else
  {
    //  compute dim At
    imAt.resize(ncols_orig,rank) ;
    for (i=0 ; i  < ncols_orig ; i++)
      for (j=0 ; j < rank ; j++)
	imAt[i][j] = a[i][j] ;

    imA.resize(nrows_orig,rank) ;
    for (i=0 ; i  < nrows_orig ; i++)
      for (j=0 ; j < rank ; j++)
	imA[i][j] = v[i][j] ;

  }

  if (DEBUG_LEVEL1)
  {
    int pb = 0;
    vpMatrix A, ApA, AAp, AApA, ApAAp ;

    nrows = nrows_orig;
    ncols = ncols_orig;

    A.resize(nrows,ncols) ;
    A = *this ;

    ApA = Ap * A;
    AApA = A * ApA;
    ApAAp = ApA * Ap;
    AAp = A * Ap;

    for (i=0;i<nrows;i++)
    {
      for (j=0;j<ncols;j++) if (fabs(AApA[i][j]-A[i][j]) > 1e-6) pb = 1;
    }
    for (i=0;i<ncols;i++)
    {
      for (j=0;j<nrows;j++) if (fabs(ApAAp[i][j]-Ap[i][j]) > 1e-6) pb = 1;
    }
    for (i=0;i<nrows;i++)
    {
      for (j=0;j<nrows;j++) if (fabs(AAp[i][j]-AAp[j][i]) > 1e-6) pb = 1;
    }
    for (i=0;i<ncols;i++)
    {
      for (j=0;j<ncols;j++) if (fabs(ApA[i][j]-ApA[j][i]) > 1e-6) pb = 1;
    }
    if (pb == 1)
    {
      printf("pb in pseudo inverse\n");
      std::cout << " A : " << std::endl << A << std::endl;
      std::cout << " Ap : " << std::endl << Ap << std::endl;
      std::cout << " A - AApA : " << std::endl << A - AApA << std::endl;
      std::cout << " Ap - ApAAp : " << std::endl << Ap - ApAAp << std::endl;
      std::cout << " AAp - (AAp)^T : " << std::endl << AAp - AAp.t() << std::endl;
      std::cout << " ApA - (ApA)^T : " << std::endl << ApA - ApA.t() << std::endl;
    }
    //    else printf("Ap OK ;-) \n");

  }


  // std::cout << v << std::endl ;
  return rank ;
}


/*!
  \brief  Return the ith rows of the matrix
  \warning notice row(1) is the 0th row.
*/

vpRowVector
vpMatrix::row(const int j)
{
  vpRowVector c(getCols()) ;

  for (int i =0 ; i < getCols() ; i++)  c[i] = (*this)[j-1][i] ;
  return c ;
}


/*!
  \brief  Return the ith columns of the matrix
  \warning notice column(1) is the 0th column.
*/

vpColVector
vpMatrix::column(const int j)
{
  vpColVector c(getRows()) ;

  for (int i =0 ; i < getRows() ; i++)     c[i] = (*this)[i][j-1] ;
  return c ;
}




/*!
  \brief Stack matrices. "Stack" two matrices  C = [ A B ]^T

  \f$ C = \left( \begin{array}{c} A \\ B \end{array}\right)    \f$

  \param A : Upper matrix.
  \param B : Lower matrix.
  \return Stacked matrix C = [ A B ]^T

  \warning A and B must have the same number of column.
*/
vpMatrix
vpMatrix::stackMatrices(const vpMatrix &A, const vpMatrix &B)
{
  vpMatrix C ;

  try{
  	stackMatrices(A,B, C) ;
  }
  catch(vpMatrixException me)
  {
    vpCERROR << me << std::endl;
    throw ;
  }

  return C ;
}

/*!
  \relates vpMatrix
  \brief stackMatrices. "stack" two matrices  C = [ A B ]^T

  \f$ C = \left( \begin{array}{c} A \\ B \end{array}\right)    \f$

  \param  A : Upper matrix.
  \param  B : Lower matrix.
  \param  C : Stacked matrix C = [ A B ]^T

  \warning A and B must have the same number of column
*/
void
vpMatrix::stackMatrices(const vpMatrix &A, const vpMatrix &B, vpMatrix &C)
{
  int nra = A.getRows() ;
  int nrb = B.getRows() ;

  if (nra !=0)
    if (A.getCols() != B.getCols())
    {
      vpERROR_TRACE("\n\t\t incorrect matrices size") ;
      throw(vpMatrixException(vpMatrixException::incorrectMatrixSizeError,
			      "\n\t\t incorrect matrices size")) ;
    }

  try {
    C.resize(nra+nrb,B.getCols()  ) ;
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    vpCERROR << me << std::endl ;
    throw ;
  }

  int i,j ;
  for (i=0 ; i < nra ; i++)
    for (j=0 ; j < A.getCols() ; j++)
      C[i][j] = A[i][j] ;


  for (i=0 ; i < nrb ; i++)
    for (j=0 ; j < B.getCols() ; j++)
    {
      C[i+nra][j] = B[i][j] ;

    }


}
/*!
  \brief Juxtapose matrices. "juxtapos" two matrices  C = [ A B ]

  \f$ C = \left( \begin{array}{cc} A & B \end{array}\right)    \f$

  \param A : Left matrix.
  \param B : Right matrix.
  \return Juxtaposed matrix C = [ A B ]

  \warning A and B must have the same number of column
*/
vpMatrix
vpMatrix::juxtaposeMatrices(const vpMatrix &A, const vpMatrix &B)
{
  vpMatrix C ;

  try{
  juxtaposeMatrices(A,B, C) ;
  }
  catch(vpMatrixException me)
  {
    vpCERROR << me << std::endl ;
    throw ;
  }

  return C ;
}

/*!
  \relates vpMatrix
  \brief juxtaposeMatrices. "juxtapose" two matrices  C = [ A B ]

  \f$ C = \left( \begin{array}{cc} A & B \end{array}\right)    \f$

  \param A : Left matrix.
  \param B : Right matrix.
  \param C : Juxtaposed matrix C = [ A B ]

  \warning A and B must have the same number of column
*/
void
vpMatrix::juxtaposeMatrices(const vpMatrix &A, const vpMatrix &B, vpMatrix &C)
{
  int nca = A.getCols() ;
  int ncb = B.getCols() ;

  if (nca !=0)
    if (A.getRows() != B.getRows())
    {
      vpERROR_TRACE("\n\t\t incorrect matrices size") ;
      throw(vpMatrixException(vpMatrixException::incorrectMatrixSizeError,
			      "\n\t\t incorrect matrices size")) ;
    }

  try {
    C.resize(B.getRows(),nca+ncb) ;
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    vpCERROR << me << std::endl ;
    throw ;
  }

  int i,j ;
  for (i=0 ; i < C.getRows(); i++)
    for (j=0 ; j < nca ; j++)
      C[i][j] = A[i][j] ;


  for (i=0 ; i < C.getRows() ; i++)
    for (j=0 ; j < ncb ; j++)
    {
      C[i][nca+j] = B[i][j] ;
    }


}
/*!

  Create a diagonal matrix with the element of a vector \f$ DA_{ii} = A_i \f$.

  \param  A : Vector which element will be put in the diagonal.

  \param  DA : Diagonal matrix DA[i][i]  = A[i]
*/

void
vpMatrix::createDiagonalMatrix(const vpColVector &A, vpMatrix &DA)
{
  int rows = A.getRows() ;
  try {
    DA.resize(rows,rows) ;
  }
  catch(vpException me)
  {
    vpERROR_TRACE("Error caught") ;
    vpCERROR << me << std::endl ;
    throw ;
  }
  DA =0 ;
  for (int i=0 ; i< rows ; i++ )
    DA[i][i] = A[i] ;
}

//--------------------------------------------------------------------
// Output
//--------------------------------------------------------------------


/*!
  \brief std::cout a matrix
*/
std::ostream &operator <<(std::ostream &s,const vpMatrix &m)
{
  s.precision(10) ;
  for (int i=0;i<m.getRows();i++) {
    for (int j=0;j<m.getCols();j++){
      s <<  m[i][j] << "  ";
    }
    s <<std::endl;
  }

  return s;
}

/*!

  Pretty print a matrix. The data are tabulated.
  The common widths before and after the decimal point
  are set with respect to the parameter maxlen.

  \param s
    Stream used for the printing.

  \param length
    The suggested width of each matrix element.
    The actual width grows in order to accomodate the whole integral part,
    and shrinks if the whole extent is not needed for all the numbers.
  \param intro
    The introduction which is printed before the matrix.
    Can be set to zero (or omitted), in which case
    the introduction is not printed.

    \return
    Returns the common total width for all matrix elements

  \sa std::ostream &operator <<(ostream &s,const vpMatrix &m)
*/
int
vpMatrix::print(std::ostream& s, unsigned length, char const* intro)
{
  typedef std::string::size_type size_type;

  int m = getRows();
  int n = getCols();

  std::vector<std::string> values(m*n);
  std::ostringstream oss;
  std::ostringstream ossFixed;
  // ossFixed <<std::fixed;
  ossFixed.setf ( std::ios::fixed, std::ios::floatfield );

  size_type maxBefore=0;  // the length of the integral part
  size_type maxAfter=0;   // number of decimals plus
                          // one place for the decimal point
  for (int i=0;i<m;++i) {
    for (int j=0;j<n;++j){
      oss.str("");
      oss << (*this)[i][j];
      if (oss.str().find("e")!=std::string::npos){
        ossFixed.str("");
        ossFixed << (*this)[i][j];
        oss.str(ossFixed.str());
      }

      values[i*n+j]=oss.str();
      size_type thislen=values[i*n+j].size();
      size_type p=values[i*n+j].find('.');

      if (p==std::string::npos){
        maxBefore=std::max(maxBefore, thislen);
        // maxAfter remains the same
      } else{
        maxBefore=std::max(maxBefore, p);
        maxAfter=std::max(maxAfter, thislen-p-1);
      }
    }
  }

  size_type totalLength=length;
  // increase totalLength according to maxBefore
  totalLength=std::max(totalLength,maxBefore);
  // decrease maxAfter according to totalLength
  maxAfter=std::min(maxAfter, totalLength-maxBefore);
  if (maxAfter==1) maxAfter=0;

  // the following line is useful for debugging
  std::cerr <<totalLength <<" " <<maxBefore <<" " <<maxAfter <<"\n";

  if (intro) s <<intro;
  s <<"["<<m<<","<<n<<"]=\n";

  for (int i=0;i<m;i++) {
    s <<"  ";
    for (int j=0;j<n;j++){
      size_type p=values[i*n+j].find('.');
      s.setf(std::ios::right, std::ios::adjustfield);
      s.width(maxBefore);
      s <<values[i*n+j].substr(0,p).c_str();

      if (maxAfter>0){
        s.setf(std::ios::left, std::ios::adjustfield);
        if (p!=std::string::npos){
          s.width(maxAfter);
          s <<values[i*n+j].substr(p,maxAfter).c_str();
        } else{
          assert(maxAfter>1);
          s.width(maxAfter);
          s <<".0";
        }
      }

      s <<' ';
    }
    s <<std::endl;
  }

  return (int)(maxBefore+maxAfter);
}


/*!
  \brief Print using matlab syntax, to be put in matlab later.

  Print using the following form:
     [ a,b,c;
       d,e,f;
       g,h,i]
*/
std::ostream & vpMatrix::
matlabPrint(std::ostream & os)
{

  int i,j;

  os << "[ ";
  for (i=0; i < this->getRows(); ++ i)
    {
      for (j=0; j < this ->getCols(); ++ j)
	{
	  os <<  (*this)[i][j] << ", ";
	}
      if (this ->getRows() != i+1) { os << ";" << std::endl; }
      else { os << "]" << std::endl; }
    }
  return os;
};

/*!
  \brief Print to be used as part of a C++ code later.

  Print under the following form:
    vpMatrix A(6,4);
    A[0][0]  = 1.4;
    A[0][1] = 0.6; ...

  \param os: the stream to be printed in.
  \param matrixName: name of the matrix, "A" by default, to be used for
  the line vpMatrix A(6,7) (see example).
  \param octet: if false, print using double, if true, print byte per byte
  each bytes of the double array.
*/
std::ostream & vpMatrix::
cppPrint(std::ostream & os, const char * matrixName, bool octet)
{

  int i,j;
  const char defaultName [] = "A";
  if (NULL == matrixName)
    {
      matrixName = defaultName;
    }
  os << "vpMatrix " << defaultName
     << " (" << this ->getRows ()
     << ", " << this ->getCols () << "); " <<std::endl;

  for (i=0; i < this->getRows(); ++ i)
    {
      for (j=0; j < this ->getCols(); ++ j)
	{
 	  if (! octet)
	    {
	      os << defaultName << "[" << i << "][" << j
		 << "] = " << (*this)[i][j] << "; " << std::endl;
	    }
	  else
	    {
	      for (unsigned int k = 0; k < sizeof(double); ++ k)
		{
		  os << "((unsigned char*)&(" << defaultName
		     << "[" << i << "][" << j << "]) )[" << k
		     <<"] = 0x" <<std::hex<<
		    (unsigned int)((unsigned char*)& ((*this)[i][j])) [k]
		     << "; " << std::endl;
		}
	    }
	}
      os << std::endl;
    }
  return os;
};





double
vpMatrix::det33(const vpMatrix &M)
{

  if ((M.getCols() !=3 ) || (M.getRows() !=3))
  {
    vpTRACE("matrix is not of size 3x3 ") ;
    throw(vpMatrixException(vpMatrixException::incorrectMatrixSizeError,
			    "\n\t\tmatrix is not of size 3x3"
			    )) ;
  }
  double detint ;

  detint = 0.0 ;
  detint =          M[0][0]*M[1][1]*M[2][2]*0.5 ;
  detint = detint + M[2][0]*M[0][1]*M[1][2]*0.5 ;
  detint = detint + M[0][2]*M[2][1]*M[1][0]*0.5 ;
  detint = detint - M[0][2]*M[1][1]*M[2][0]*0.5 ;
  detint = detint - M[0][0]*M[2][1]*M[1][2]*0.5 ;
  detint = detint - M[2][2]*M[1][0]*M[0][1]*0.5 ;
  return(detint);

}







/*!
  \return the norm if the matrix is initialized, 0 otherwise
  \sa infinityNorm
*/
double
vpMatrix::euclidianNorm () const
{
  double norm=0.0;
  double x ;
  for (int i=0;i<dsize;i++)
    { x = *(data +i); norm += x*x;  }

  return norm;
}



/*!
  \return the norm if the matrix is initialized, 0 otherwise
  \sa euclidianNorm
*/
double
vpMatrix::infinityNorm () const
{
  double norm=0.0;
  double x ;
  for (int i=0;i<dsize;i++)
    {
      x = fabs (*(data + i)) ;
      if (x > norm) { norm = x; }
    }

  return norm;
}




#undef DEBUG_LEVEL1
/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */
