#ifndef MYFUNCTIONS_H
#define MYFUNCTIONS_H

#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_out.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/grid/grid_refinement.h>
#include <deal.II/grid/grid_in.h>
#include <deal.II/base/point.h>
#include <deal.II/base/quadrature_lib.h>
#include <deal.II/base/timer.h>
#include <deal.II/base/convergence_table.h>
#include <deal.II/base/parameter_handler.h>
#include <deal.II/base/utilities.h>
#include <deal.II/base/function.h>
#include <deal.II/base/logstream.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_tools.h>
#include <deal.II/dofs/dof_accessor.h>
#include <deal.II/dofs/dof_renumbering.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/compressed_sparsity_pattern.h>
#include <deal.II/lac/sparse_direct.h>
#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/compressed_sparsity_pattern.h>
#include <deal.II/lac/solver_cg.h>
#include <deal.II/lac/precondition.h>
#include <deal.II/lac/constraint_matrix.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/numerics/vector_tools.h>
#include <deal.II/numerics/matrix_tools.h>
#include <deal.II/numerics/data_out.h>
#include <deal.II/numerics/error_estimator.h>
#include <deal.II/numerics/solution_transfer.h>

#include <fstream>
#include <cmath>


using namespace dealii;

/*******************************************************************************
 * Define zero function
 ******************************************************************************/
template<int dim>
class MyZeroFunction : public Function<dim>
{
    public:
        MyZeroFunction() : Function<dim>(3) {};       
        virtual void vector_value(const Point<dim> &p, 
				Vector<double> &values) const;
};

template<int dim>
void MyZeroFunction<dim>::vector_value(const Point<dim> &p, 
                Vector<double> &values) const
{
	values(0) = 0;
	values(1) = 0;
	values(2) = 0;
}


/*******************************************************************************
 * Define boundary conditions, initial conditions, forcing function and 
 * exact solution for convergence study
 ******************************************************************************/
template<int dim>
class ExactSolutionBoundaryValues : public Function<dim>
{
    public:
        ExactSolutionBoundaryValues() : Function<dim>(3) {}
        virtual void vector_value(const Point<dim> &p, 
                Vector<double> &values) const;
};

template<int dim>
void ExactSolutionBoundaryValues<dim>::vector_value(const Point<dim> &p, 
                Vector<double> &values) const
{  	
	const double t = this->get_time();
	
	values(0) = -exp(-t)*cos(M_PI*p[0])/M_PI;
	values(1) = -exp(-t)*p[1]*sin(M_PI*p[0]);
	values(2) = 0; 
 }
 
template<int dim>
class ExactSolutionInitialCondition : public Function<dim>
{
    public:
        ExactSolutionInitialCondition() : Function<dim>(3) {};       
        virtual void vector_value(const Point<dim> &p, 
				Vector<double> &values) const;
};

template<int dim>
void ExactSolutionInitialCondition<dim>::vector_value(const Point<dim> &p, 
                Vector<double> &values) const
{
	values(0) = -cos(M_PI*p[0])/M_PI;
	values(1) = -p[1]*sin(M_PI*p[0]);
	values(2) = 0;
}

template<int dim>
class ExactSolutionForcingFunction : public Function<dim>
{
    public:
        ExactSolutionForcingFunction() : Function<dim>(3) {};       
        virtual void vector_value(const Point<dim> &p, 
				Vector<double> &values) const;
};

template<int dim>
void ExactSolutionForcingFunction<dim>::vector_value(const Point<dim> &p, 
                Vector<double> &values) const
{
	const double t = this->get_time();
	
	values(0) = cos(M_PI*p[0])*exp(-t)*(1/M_PI - M_PI) + 2*p[0]*pow(p[1],2);
	values(1) = sin(M_PI*p[0])*exp(-t)*p[1]*(1 - pow(M_PI,2)) + 2*pow(p[0],2)*p[1];
	values(2) = 0;
}

template<int dim>
class ExactSolution : public Function<dim>
{
    public:
        ExactSolution() : Function<dim>(3) {}
        virtual void vector_value(const Point<dim> &p, 
                Vector<double> &values) const;
        virtual void vector_gradient(const Point<dim> &p, 
                std::vector<Tensor<1,dim> > &gradients) const;
};

template<int dim>
void ExactSolution<dim>::vector_value(const Point<dim> &p, 
                Vector<double> &values) const
{
	const double t = this->get_time();
	
    values(0) = -exp(-t)*cos(M_PI*p[0])/M_PI;
	values(1) = -exp(-t)*p[1]*sin(M_PI*p[0]);
	values(2) = pow(p[0]*p[1],2); 
}

template<int dim>
void ExactSolution<dim>::vector_gradient(const Point<dim> &p, 
                std::vector<Tensor<1,dim> > & gradients) const
{
	const double t = this->get_time();
	
    gradients[0][0] = exp(-t)*sin(M_PI*p[0]);
    gradients[0][1] = 0;
    
    gradients[1][0] = -exp(-t)*M_PI*p[1]*cos(M_PI*p[0]);
    gradients[1][1] = -exp(-t)*sin(M_PI*p[0]);
    
    gradients[2][0] = 2*p[0]*pow(p[1],2);
    gradients[2][1] = 2*p[1]*pow(p[0],2);
}

/*******************************************************************************
 * Define boundary conditions and forcing functions for lid driven cavity
 ******************************************************************************/
template<int dim>
class DrivenCavityBoundaryValues : public Function<dim>
{
    public:
        DrivenCavityBoundaryValues() : Function<dim>(3) {}
        virtual void vector_value(const Point<dim> &p, 
                Vector<double> &values) const;
};

template<int dim>
void DrivenCavityBoundaryValues<dim>::vector_value(const Point<dim> &p, 
                Vector<double> &values) const
{  
	values(0) = (std::fabs(p[1]-1.0) < 1.0e-14 ? 1 : 0);
	values(1) = 0;
	values(2) = 0;     
}

/*******************************************************************************
 * Define boundary conditions, initial conditions, forcing function for a 
 * test case for adaptive meshing
 ******************************************************************************/
template<int dim>
class AdaptiveBoundaryValues : public Function<dim>
{
    public:
        AdaptiveBoundaryValues() : Function<dim>(3) {}
        virtual void vector_value(const Point<dim> &p, 
                Vector<double> &values) const;
};

template<int dim>
void AdaptiveBoundaryValues<dim>::vector_value(const Point<dim> &p, 
                Vector<double> &values) const
{  	
	const double t = this->get_time();
	
	if (std::fabs(p[1]) < 1.0e-14) //bottom boundary
	{
		values(0) = sin(2*M_PI*t + M_PI/2);
		values(1) = 0;
	}
	else if (std::fabs(p[0] - 1) < 1.0e-14) //right boundary
	{	
		values(0) = 0;
		values(1) = 0;
	}
	else if (std::fabs(p[1] - 1) < 1.0e-14) //top boundary
	{
		values(0) = sin(2*M_PI*t);
		values(1) = 0;
	}
	else //left boundary
	{
		values(0) = 0;
		values(1) = 0;
	}
	
	values(2) = 0; 
 }
 
template<int dim>
class AdaptiveForcingFunction : public Function<dim>
{
    public:
        AdaptiveForcingFunction() : Function<dim>(3) {}
        virtual void vector_value(const Point<dim> &p, 
                Vector<double> &values) const;
};

template<int dim>
void AdaptiveForcingFunction<dim>::vector_value(const Point<dim> &p, 
                Vector<double> &values) const
{  	
	const double t = this->get_time();
	
	if (p[0] < t)
	{
		values(0) = 1;
	}
	else
	{
		values(0) = 0;
	}
	
	values(1) = 0;
	values(2) = 0;
}

 
template<int dim>
class AdaptiveInitialConditions : public Function<dim>
{
    public:
        AdaptiveInitialConditions() : Function<dim>(3) {}
        virtual void vector_value(const Point<dim> &p, 
                Vector<double> &values) const;
};

template<int dim>
void AdaptiveInitialConditions<dim>::vector_value(const Point<dim> &p, 
                Vector<double> &values) const
{  	
	const double t = this->get_time();
	
	values(0) = 10000;
	values(1) = 10;
	values(2) = 0;
}
 
 
#endif
