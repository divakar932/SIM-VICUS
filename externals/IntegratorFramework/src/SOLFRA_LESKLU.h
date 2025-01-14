#ifndef SOLFRA_LESKLUH
#define SOLFRA_LESKLUH

#include "SOLFRA_LESInterfaceDirect.h"

namespace SOLFRA {

class JacobianSparseCSR;

/*! Implementation of a sparse KLU equation system solver. */
class LESKLU : public LESInterfaceDirect {
public:
	/*! Sparse KLU equation system constructor. */
	LESKLU();

	/*! Destructor, releases memory. */
	~LESKLU() override;

	/*! Re-implemented from LESInterface::init(). 	*/
	void init(ModelInterface * model, IntegratorInterface * integrator,
			  PrecondInterface * precond, JacobianInterface * jacobian) override;

	/*! Re-implemented from LESInterface::setup(). 	*/
	virtual void setup(const double * /*y*/, const double * /*ydot*/, const double * /*residuals*/, double /*gamma*/) override { }

	/*! Re-implemented from LESInterface::solve(). */
	virtual void solve(double * /*rhs*/) override { }

	/*! Re-implemented from LESInterface::updateSUNDIALSStatistics().
		Updates statistics counters m_statNumJacEvals and m_statNumRhsEvals before calling LESInterfaceDirect::writeMetrics().
	*/
	virtual void updateSUNDIALSStatistics() override;

private:
	/*! Sparse matrix implementation (not owned by us). */
	JacobianSparseCSR						*m_jacobian;
};

} // namespace SOLFRA


#endif // SOLFRA_LESKLUH
