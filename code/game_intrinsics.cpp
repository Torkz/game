
namespace game
{
namespace bit
{
	struct scan_bit_result
	{
		bool found;
		uint32 index;
	};

	inline internal
	scan_bit_result find_least_significant_set_bit(uint32 value)
	{
		scan_bit_result result = {};

#if COMPILER_MSVC
		result.found = _BitScanForward((unsigned long*)&result.index, value);
#else
		for(uint32 i=0;
			i<32;
			++i)
		{
			if((value >> i) & 1)
			{
				result.found = true;
				result.index = i;
				break;
			}
		}
#endif

		return result;
	}
} //namespace bit
} //namespace game