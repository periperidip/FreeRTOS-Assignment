using JuMP, Gurobi

# ILP to compute a static schedule of the given jobs

### Given:
# Periodic tasks (Period/Deadline, Execution Time):
# T1 = (100, 10)
# T2 = (200, 11)
# T3 = (150, 15)
# T4 = (100, 12)
#
# Hyperperiod: LCM(P1, P2, P3, P4) = 600

# The Jobset, NOTE: J1i is the ith job of first task
jobs = [:J11, :J12, :J13, :J14, :J15, :J16, :J21, :J22, :J23, :J31,
		:J32, :J33, :J34, :J41, :J42, :J43, :J44, :J45, :J46]

# Release times for each job
releasetimes = Dict(
    :J11 => 0,
    :J12 => 100,
    :J13 => 200,
    :J14 => 300,
    :J15 => 400,
    :J16 => 500,
    :J21 => 0,
    :J22 => 200,
    :J23 => 400,
    :J31 => 0,
    :J32 => 150,
	:J33 => 300,
	:J34 => 450,
	:J41 => 0,
    :J42 => 100,
    :J43 => 200,
    :J44 => 300,
    :J45 => 400,
    :J46 => 500
)

# Deadlines for each job
deadlines = Dict(
	:J11 => 100,
    :J12 => 200,
    :J13 => 300,
    :J14 => 400,
    :J15 => 500,
    :J16 => 600,
    :J21 => 200,
    :J22 => 400,
    :J23 => 600,
    :J31 => 150,
    :J32 => 300,
	:J33 => 450,
	:J34 => 600,
	:J41 => 100,
    :J42 => 200,
    :J43 => 300,
    :J44 => 400,
    :J45 => 500,
    :J46 => 600
)

# Execution times for each job
executiontimes = Dict(
	:J11 => 10,
    :J12 => 10,
    :J13 => 10,
    :J14 => 10,
    :J15 => 10,
    :J16 => 10,
    :J21 => 11,
    :J22 => 11,
    :J23 => 11,
    :J31 => 15,
    :J32 => 15,
	:J33 => 15,
	:J34 => 15,
	:J41 => 12,
    :J42 => 12,
    :J43 => 12,
    :J44 => 12,
    :J45 => 12,
    :J46 => 12
)

# Invoking the Gurobi Optimizer. The invocations throw errors.
# Here are the different invocations we tried.
#########

# const GRB_ENV = Gurobi.Env()
# m = Model(with_optimizer(Gurobi.Optimizer, GRB_ENV))

# m = Model(solver=GurobiSolver())
# m = direct_model(Gurobi.Optimizer())

m = JuMP.Model(Gurobi.Optimizer)
set_optimizer_attribute(m, "TimeLimit", 500)
set_optimizer_attribute(m, "Presolve", 0)

#########

# Variables for job start times
@variable(m, x[j = jobs])

# CONSTRAINT: Job Start time MUST be after Job Release time
for job in jobs
    @constraint(m, x[job] >= releasetimes[job])
end

# CONSTRAINT: Job must end before its deadline
for job in jobs
    @constraint(m, x[job] + executiontimes[job] <= deadlines[job])
end

# CONSTRAINT: for each pair of jobs (J1,J2), their execution
# must not overlap in time. This can be expressed as a disjunction (OR):
#
# J1 after J2: start of J1 >= start of J2 + execution time of J2
#   OR
# J1 before J2: start of J2 >= start of J1 + execution time of J1
# 
# Such disjunctions are harder to express in ILPs. We can use the Big-M method
# using a binary helper variable and a sufficiently large constant BigM.
@variable(m, helper[job1=jobs,job2=jobs], Bin)
# Left-hand side is always smaller than this value.
BigM = 2*36

for j1 in jobs
    for j2 in jobs
        if j1 != j2
            @constraint(m, x[j2] + executiontimes[j2] - x[j1] <= BigM * helper[j1,j2])
	    @constraint(m, x[j1] + executiontimes[j1] - x[j2] <= BigM * (1 - helper[j1,j2]))
        end
    end
end

# The invocation of the solve function throws an error.
# Here are the different variations we tried.
#########

# JuMP.solve(m)

# solve(m)

optimize!(m)

#########

sol = getvalue(x)

for job in jobs
    println(string(job) * " -> " * string(sol[job]));
end
