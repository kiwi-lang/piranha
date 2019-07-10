#ifndef PIRANHA_SINGLE_OUTPUT_NODE_H
#define PIRANHA_SINGLE_OUTPUT_NODE_H

#include "node.h"

#include <sstream>

namespace piranha {

	template <typename OutputType>
	class PipeNode : public Node {
	public:
		PipeNode() {
			/* void */
		}

		~PipeNode() {
			/* void */
		}

		virtual void _initialize() {
			m_output.initialize();
		}

		virtual void _evaluate() {
			/* void */
		}

		virtual void _destroy() {
			/* void */
		}

		virtual void registerOutputs() {
			setPrimaryOutput(&m_output);
			registerOutput(&m_output, "$primary");
		}

		virtual void registerInputs() {
			int inputCount = m_output.getInputCount();
			for (int i = 0; i < inputCount; i++) {
				// Generate a standard name
				std::stringstream ss;
				if (inputCount > 1) ss << "__in" << i;
				else ss << "__in";

				// Register all inputs of the one node output
				registerInput(m_output.NodeOutput::getInputConnection(i), ss.str());
			}
		}

	protected:
		OutputType m_output;
	};

} /* namespace piranha */

#endif /* PIRANHA_SINGLE_OUTPUT_NODE_H */
