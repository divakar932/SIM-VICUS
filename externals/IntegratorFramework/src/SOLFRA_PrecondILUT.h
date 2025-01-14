#ifndef SOLFRA_PrecondILUTH
#define SOLFRA_PrecondILUTH

#include "SOLFRA_PrecondInterface.h"

#include <vector>

namespace IBKMK {
	class SparseMatrixCSR;
	struct SpaFmt;
	struct ILUfac;
}

namespace SOLFRA {

class IntegratorInterface;
class JacobianSparse;
class ModelInterface;

/*! ILU-t for use with Sundials solvers.
	The implementation uses ITSOL routines for ILU-t setup and solve.

	PrecondILUT needs CSR sparse matrix storage format. During init(), when there is a sparse matrix
	provided from the JacobianInterface in different format, a sparse matrix with CSR pattern will be
	created and stored locally instead.
*/
class PrecondILUT : public SOLFRA::PrecondInterface {
public:
	/*! Initializes PrecondILUT. */
	explicit PrecondILUT(PreconditionerType precondType = PrecondInterface::Right, unsigned int maxFillinLevel = 0 );

	/*! Destructor, releases band matrix memory. */
	~PrecondILUT();

	/*! Returns type of precondition (where it should be applied in context of the iteration linear equation solver). */
	virtual PreconditionerType preconditionerType() const override { return m_precondType; }

	/*! Initialize the preconditioner, called from the
		framework before integration is started. */
	virtual void init(SOLFRA::ModelInterface * model, SOLFRA::IntegratorInterface * integrator,
					  const SOLFRA::JacobianInterface * jacobianInterface) override;

	/*! In this function, the preconditioner matrix is composed an LU-factorised.
		This function is called from the linear equation solver during iterations.
		\param y The current prediction of the solution.
		\param residuals The currentl prediction of the solution.
	*/
	virtual int setup(double t, const double * y, const double * ydot, const double * residuals,
		bool jacOk, bool & jacUpdated, double gamma) override;

	virtual int solve(double t, const double * y, const double * ydot, const double * residuals,
		const double * r, double * z, double gamma, double delta, int lr) override;

	/*! Holds number of RHS function evaluations (ydot()/residual() calls) used for generating
		the preconditioner.
	*/
	virtual unsigned int nRHSEvals() const override { return 0; }

	/*! Computes and returns serialization size, by default returns 0 which means feature not supported. */
	virtual std::size_t serializationSize() const override;

	/*! Stores content at memory location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Default implementation does nothing.
	*/
	virtual void serialize(void* & dataPtr) const override;

	/*! Restores content from memory at location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Default implementation does nothing.
	*/
	virtual void deserialize(void* & dataPtr) override;

private:
	/*! Jacobian matrix, if provided, the Jacobian data is transferred within
		setup() from the Jacobian data, rather than being computed anew via DQ approximation.
		Note: can be either SOLFRA::JacobianSparseCSR or SOLFRA::JacobianSparseEID.
	*/
	const SOLFRA::JacobianSparse			*m_jacobianSparse;

	/*! Sparse matrix implementation (owned, because we use an in-place ILU operation and do not want
		to destroy the original Jacobian memory which is needed for the iterative solver).
	*/
	IBKMK::SparseMatrixCSR					*m_precondMatrixCSR;

	/*! Sparse matrix implementation in ITSOL format.
	*/
	IBKMK::SpaFmt							*m_itsolMatrix;
	/*! ILU composition of preconditioner matrix.
	*/
	IBKMK::ILUfac							*m_factorizedItsolMatrix;
	/*! Maximum fill-in level (>= 0, but only > 0 is meaningful). */
	unsigned int							m_maxLevelOfFillIn;
	/*! Tolerance for fill-in element.
	*/
	double									m_tolerance;
};

} // namespace SOLFRA

#endif // SOLFRA_PrecondILUT
