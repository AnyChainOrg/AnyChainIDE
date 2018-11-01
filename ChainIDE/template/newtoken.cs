using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using static UvmCoreLib.UvmCoreFuncs;
using UvmCoreLib;

// This is a demo. Please remove the unused code
// and add your customized code.
/*sdf
fff
sdfsdf
*/
namespace Token
{
    public class Storage
    {
        public string name { get; set; }
        public string symbol { get; set; }
        public long supply { get; set; }
        public long precision { get; set; }
        public string state { get; set; }
        public bool locked { get; set; }
        public long rate { get; set; }  //ico 兑换比例  含精度比例 
        public long icoEndBlocknum { get; set; }
        public UvmMap<int> users { get; set; }
        public string admin { get; set; }
    }

    public class MyEventEmitteer : IUvmEventEmitter
    {
        public static void EmitTransfer(string eventArg)
        {
            Console.WriteLine("event Transfer, arg is " + eventArg);
        }
        public static void EmitPaused(string eventArg)
        {
            Console.WriteLine("event Paused, arg is " + eventArg);
        }
        public static void EmitResumed(string eventArg)
        {
            Console.WriteLine("event Resumed, arg is " + eventArg);
        }
        public static void EmitStopped(string eventArg)
        {
            Console.WriteLine("event Stopped, arg is " + eventArg);
        }

        public static void EmitReceived(string eventArg)
        {
            Console.WriteLine("event Stopped, arg is " + eventArg);
        }




    }

    public class MyContract : UvmContract<Storage>
    {
        public MyContract() : base(new Storage())
        {
        }
        public override void init()
        {
            this.storage.name = "";
            this.storage.symbol = "";
            this.storage.supply = 0;
            this.storage.precision = 0;
            this.storage.state = "NOT_INITED";
            this.storage.admin = caller_address();
            this.storage.locked = false;
            this.storage.rate = 0;
            this.storage.icoEndBlocknum = 0;
            this.storage.users = UvmMap<int>.Create();
        }

        public void checkAdmin()
        {
            if (this.storage.admin != caller_address())
            {
                Error("you are not admin, can't call this function");
            }
        }

        public void checkState()
        {
            if (this.storage.state != "COMMON")
            {
                Error("not allowed when state is " + this.storage.state);
            }
        }


        public void checkLock()
        {
            if (this.storage.locked)
            {
                Error("not allowed when locked ");
            }
        }


        public long getRate()
        {
            return this.storage.rate;
        }

        public long getIcoEndBlocknum()
        {
            return this.storage.icoEndBlocknum;
        }

        public long getSupply()
        {
            return this.storage.supply;
        }

        public string getAdmin()
        {
            return this.storage.admin;
        }

        public long getPrecision()
        {
            return this.storage.precision;
        }

        public string getName()
        {
            return this.storage.name;
        }

        public string getSymbol()
        {
            return this.storage.symbol;
        }

        public bool getLocked()
        {
            return this.storage.locked;
        }

        public string getState()
        {
            return this.storage.state;
        }

        public override void on_deposit(long amount)
        {
            checkState();
            var prev_contract_id = get_prev_call_frame_contract_address();
            if (Type(prev_contract_id) != "nil" && tostring(prev_contract_id) != "")
            {
                var preid = tostring(prev_contract_id);
                Error("this api can't called by contract:" + preid);
            }

            var remain = fast_map_get("users", this.storage.admin);
            var headerblocknum = get_header_block_num();
            if (headerblocknum >= this.storage.icoEndBlocknum)
            {
                Error("ico has ended");
            }
            if (amount <= 0)
            {
                Error("amount should greater than 0");
            }
            long remainamount = Type(remain) == "number" ? tointeger(remain) : 0;
            if (remainamount <= 0)
            {
                Error("no tokens left");
            }
            long wantamount = amount * this.storage.rate;
            if ((wantamount / this.storage.rate) != amount)
            {
                Error("invalid amount");
            }
            if (remainamount < wantamount)
            {
                Error("not enlough tokens");
            }

            var calleraddress = caller_address();
            var balance = fast_map_get("users", calleraddress);
            long toBalance = (Type(balance) == "number") ? tointeger(balance) : 0;

            if (wantamount <= 0 ||  toBalance + wantamount < toBalance || toBalance < 0)
            {
                Error("invalid amount");
            }

            fast_map_set("users", this.storage.admin, remainamount - wantamount);
            fast_map_set("users", caller_address(), toBalance + wantamount);

            var eventarg = caller_address() + ",";
            eventarg = eventarg + tostring(amount);
            eventarg = eventarg + tostring(wantamount);
            MyEventEmitteer.EmitReceived(eventarg);

        }

        //arg: name,symbol,supply,precision
        public void init_token(string arg)
        {
            checkAdmin();
            if (this.storage.state != "NOT_INITED")
            {
                Error("this token contract inited before");
            }
            this.storage.supply = 300000000000;
            this.storage.name = "ABC TOKEN";
            this.storage.symbol = "ABC";
            this.storage.precision = 10000;
            this.storage.state = "COMMON";
            this.storage.rate = 20;
            this.storage.icoEndBlocknum = 50000;
            fast_map_set("users", this.storage.admin, this.storage.supply);
        }

        public long getBalance(string addr)
        {
            var balance = fast_map_get("users", addr);
            if (Type(balance) != "number")
            {
                return 0;
            }
            return tointeger(balance);
        }

        public long getBalance2(string addr)
        {
            return this.storage.users.Get(addr);
        }

        public void setLock(long locked)
        {
            checkAdmin();
            if (locked == 0)
            {
                this.storage.locked = false;
            }
            else
            {
                this.storage.locked = true;
            }
        }

        public void adminWithdraw()
        {
            checkAdmin();
            print("adminWithdraw");
            var assetname = get_system_asset_symbol();
            var amount = get_contract_balance_amount(get_current_contract_address(), assetname);
            var res = transfer_from_contract_to_address(this.storage.admin, assetname, amount);
            if (res != 0)
            {
                Error("transfer_from_contract_to_address fail amount=" + tostring(amount));
            }
            
        }



        //arg format json : {"toaddress":"xxxxxxxxxxxxxxxxx","amount":300}
        public void transfer(string args)
        {
            checkState();

            var prev_contract_id = get_prev_call_frame_contract_address();
            if (Type(prev_contract_id) != "nil" && tostring(prev_contract_id)!="")
            {
                var preid = tostring(prev_contract_id);
                Error("this api can't called by contract:" + preid);
            }

            var headerblocknum = get_header_block_num();
            //if (headerblocknum < this.storage.icoEndBlocknum)
            //{
            //    Error("ico not end yet , not allowed transfer ");
            //}
            var jsonModule = importModule<UvmJsonModule>("json");
            var jsonargs = (UvmMap<string>)jsonModule.Loads(args);

            string to_address = tostring(jsonargs.Get("toaddress"));
            long amount = tointeger(jsonargs.Get("amount"));

            if (!is_valid_address(to_address))
            {
                Error("invalid toaddress");
            }

            string concaller = caller_address();
            var balance = fast_map_get("users", concaller);

            long fromBalance = (Type(balance) == "number") ? tointeger(balance) : 0;

            balance = fast_map_get("users", to_address);
            long toBalance = (Type(balance) == "number") ? tointeger(balance) : 0;

            if (amount <= 0 || fromBalance < amount || toBalance + amount < toBalance || toBalance < 0)
            {
                Error("invalid amount");
            }
            fromBalance = fromBalance - amount;
            toBalance = toBalance + amount;
            fast_map_set("users", concaller, fromBalance);
            fast_map_set("users", to_address, toBalance);

            var eventarg = to_address + ",";
            eventarg = eventarg + tostring(amount);
            MyEventEmitteer.EmitTransfer(to_address);

        }

        public void on_destory(string args)
        {
            Error("not allowed destory");
        }
    }

    public class ExampleLibClass
    {
        public MyContract Main()
        {
            print("start of demo C# contract");
            var contract = new MyContract();
            print("end main");
            return contract;
        }
    }
}