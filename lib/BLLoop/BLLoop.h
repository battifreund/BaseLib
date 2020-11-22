#ifndef _BLLOOP_H_
#define _BLLOOP_H_

namespace BL
{
    class Loopable
    {
        protected:
            int interval = 0;

        public:
            Loopable();
            
            void loop();

            void setLoopInterval(int ms);
    };
}; // namespace BL

#endif