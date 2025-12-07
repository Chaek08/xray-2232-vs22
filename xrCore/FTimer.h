#ifndef FTimerH
#define FTimerH
#pragma once

class CTimer_paused;

class XRCORE_API pauseMngr
{
	xr_vector<CTimer_paused*>	m_timers;
	BOOL						m_paused;
public:
	pauseMngr():m_paused(FALSE){}
	BOOL Paused(){return m_paused;};
	void Pause(BOOL b);
	void Register (CTimer_paused* t);
	void UnRegister (CTimer_paused* t);
};

extern XRCORE_API pauseMngr		g_pauseMngr;

class XRCORE_API CTimer {
protected:
	u64			qwStartTime;
	u64			qwPausedTime;
	u64			qwPauseAccum;
	float		fResult;
	BOOL		bPause;

	float		m_time_factor;
	u64			m_real_clocks;
	u64			m_clocks;

protected:
	
	ICF u64		get_raw_clocks() const
	{
		if (bPause)
			return qwPausedTime;
		else
			return CPU::GetCycleCount() - qwStartTime - CPU::cycles_overhead - qwPauseAccum;
	}

	IC u64		apply_time_factor(const u64 &current_raw) const
	{
		u64   delta       = current_raw - m_real_clocks;
		double delta_d    = (double)delta;
		double tf_d       = (double)m_time_factor;
		double scaled_d   = delta_d * tf_d + 0.5;
		u64   scaled      = (u64)scaled_d;
		return m_clocks + scaled;
	}

public:
				CTimer()
					: qwStartTime(0)
					, qwPausedTime(0)
					, qwPauseAccum(0)
					, fResult(0.f)
					, bPause(FALSE)
					, m_time_factor(1.f)
					, m_real_clocks(0)
					, m_clocks(0)
				{
				}

	IC	void	Start()
	{
		if (bPause)
			return;

		qwStartTime = CPU::GetCycleCount() - qwPauseAccum;

		m_real_clocks = 0;
		m_clocks      = 0;
	}

	IC	float	Stop()			{	return (fResult = GetElapsed_sec());							}
	IC	float	Get	()			{	return fResult;													}

	ICF u64		GetElapsed_clk() const
	{
#ifndef _EDITOR
		FPU::m64r();
#endif
		u64 raw = get_raw_clocks();
		u64 res = apply_time_factor(raw);
#ifndef _EDITOR
		FPU::m24r();
#endif
		return res;
	}

	IC	u32		GetElapsed_ms() const
	{
		return u32(u64(GetElapsed_clk()) / u64(CPU::cycles_per_milisec));
	}

	IC	float	GetElapsed_sec() const
	{
#ifndef _EDITOR
		FPU::m64r();
#endif
		float result = float(double(GetElapsed_clk()) * CPU::cycles2seconds);
#ifndef _EDITOR
		FPU::m24r();
#endif
		return result;
	}

	IC	void	Dump()
	{
		Msg("* Elapsed time (sec): %f", GetElapsed_sec());
	}

	IC	const float& time_factor() const
	{
		return m_time_factor;
	}

	IC	void time_factor(const float &time_factor)
	{
		u64 current = get_raw_clocks();
		m_clocks    = apply_time_factor(current);
		m_real_clocks = current;
		m_time_factor = time_factor;
	}
};

class XRCORE_API CTimer_paused  : public CTimer		{
	u64							save_clock;
public:
	CTimer_paused				()		{ g_pauseMngr.Register(this);	}
	~CTimer_paused				()		{ g_pauseMngr.UnRegister(this);	}
	IC BOOL		Paused			()		{ return bPause;				}
	IC void		Pause			(BOOL b){
		if(bPause==b)return;

		if( b ){
			save_clock			= CPU::GetCycleCount()-CPU::cycles_overhead;
			qwPausedTime		= get_raw_clocks();
		}else{
			u64 cur_clock		= CPU::GetCycleCount()-CPU::cycles_overhead;
			qwPauseAccum		+=  cur_clock - save_clock;
		}
		bPause = b;
	}
};

class XRCORE_API CStatTimer
{
public:
	CTimer		T;
	__int64		accum;
	float		result;
	u32			count;
public:
				CStatTimer		();
	void		FrameStart		();
	void		FrameEnd		();

	ICF void	Begin			()		{	count++; T.Start(); }
	ICF void	End				()		{	accum += T.GetElapsed_clk(); }

	ICF u64		GetElapsed_clk	()		{	return accum; }
	ICF u32		GetElapsed_ms	()		{	return u32(u64(GetElapsed_clk())/u64(CPU::cycles_per_milisec)); }
	ICF float	GetElapsed_sec	()		{	return float(GetElapsed_clk())*CPU::cycles2seconds; }
	ICF float	GetFrame_sec	()		{	return result; }
};

#endif // FTimerH
