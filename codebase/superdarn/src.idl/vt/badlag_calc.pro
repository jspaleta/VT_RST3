;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;+
;NAME:
; badlag_calc
;
; PURPOSE:
; 	Plots pulse sequence and badlag information
;
; CATEGORY:
; 	Graphics
;
; CALLING SEQUENCE:
; 	badlag_calc,seq,mpinc,lagfr,nrang,nblnk,time
;
;	INPUTS:
;		seq: which pulse sequence to use, 0=katscan, 1=old normalscan, 2=tauscan
;		mpinc: the number of samples per fundamental lag
;		lagfr: lag to first range in samples
;		nrang: number of range gates
;		nblnk: number of samples blanked for each pulse
;		time:  a string with a timestamp to be used for a file name
;
; OPTIONAL INPUTS:
;
; KEYWORD PARAMETERS:
;
; EXAMPLE:
;		badlag_calc,0,5,4,100,3,011112
;
; OUTPUT:
; 	/rst/output_plots/011112.badlags.ps
;
;
; COPYRIGHT:
; Copyright (C) 2011 by Virginia Tech
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
; THE SOFTWARE.
;
;
; MODIFICATION HISTORY:
; Written by AJ Ribeiro 03/06/2012
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

pro badlag_calc,seq,mpinc,lagfr,nrang,nblnk,time

  set_plot,'PS',/copy
  device,/landscape,/COLOR,BITS_PER_PIXEL=8,filename='/rst/output_plots/'+time+'.badlags.ps'
  
	;set information for katscan
	if(seq eq 0) then begin
		npul = 8
		nlags = 23
		name='katscan'
		;pulse sequence
		pulse = [0,14,22,24,27,31,42,43]
		;lag table
		lagt1 = [0,42,22,24,27,22,24,14,22,14,31,31,14,0, 27,27,14,24,24,22,22,0, 0]
		lagt2 = [0,43,24,27,31,27,31,22,31,24,42,43,27,14,42,43,31,42,43,42,43,22,24]
		lag_avail = lagt2-lagt1
	endif

	;calculate the number of samples
	nsamp = pulse(npul-1)*mpinc+lagfr+nrang
	samples = intarr(nsamp)
	;calculate the bad samples
	for i=0,npul-1 do begin
		for j=0,nblnk-1 do begin
			samples(pulse(i)*mpinc+j) = 1
		endfor
	endfor
	loadct,0

	title = name+' mpinc='+strtrim(mpinc,2)+' lagfr='+strtrim(lagfr,2)+' nrang='+strtrim(nrang,2)+' nblnk='+strtrim(nblnk,2)

	;first, plot the pulse sequence
	plot,findgen(1),findgen(1),xrange=[0,nsamp],yrange=[0,.75],$
				ystyle=1,xstyle=1,yticks=1,/nodata,/noerase,pos=[.05,.725,.95,.95],$
				title='Pulse sequence for '+title,ytickname=replicate(' ',3),xtitle='time (samples)',$
				charthick=3.,xthick=3.,ythick=3.,xticklen=-.02
	for i=0,npul-1 do begin
		if(i MOD 2 eq 0) then y = .55 $
		else y = .62
		polyfill,[pulse(i)*mpinc,pulse(i)*mpinc+nblnk,pulse(i)*mpinc+nblnk,pulse(i)*mpinc],[0,0,.5,.5],/data
		xyouts,mean([pulse(i)*mpinc,pulse(i)*mpinc+nblnk]),y,strtrim(pulse(i),2),/data,charthick=3.,align=.5,charsize=.7
	endfor
	;next, plot the samples
	plot,findgen(1),findgen(1),xrange=[0,nsamp],yrange=[0,.75],$
				ystyle=1,xstyle=1,yticks=1,/nodata,/noerase,pos=[.05,.4,.95,.625],$
				title='Samples for '+title,ytickname=replicate(' ',3),xtitle='time (samples)',$
				charthick=3.,xthick=3.,ythick=3.,xticklen=-.02
	loadct,34
	cnt = 0
	for i=0,nsamp-1 do begin
		if(samples(i) eq 0) then color = 150 $
		else color = 255
		plots,[i,i],[0,.5],col=color,thick=3.,linestyle=0
		x = where(pulse eq (double(i)/double(mpinc)))
		if(x ge 0) then begin
			print,x
			loadct,0
			if(x MOD 2 eq 0) then y = .55 $
			else y = .62
			xyouts,mean([i,i+nblnk-1]),y,'['+strtrim(i,2)+','+strtrim(i+nblnk-1,2)+']',/data,charthick=3.,align=.5,charsize=.7
; 			cnt = cnt + 1
			loadct,34
		endif
	endfor

	loadct,0
	;next plot the lag table
	xyouts,.5,.31,'Lag table for '+title,align=.5,charthick=3.,/normal,charsize=1.3
	plots,[.05,.95],[.3,.3],/normal,thick=3.

	if(nlags MOD 2 eq 0) then x_cells = nlags/2+1 $
	else x_cells = (nlags+1)/2+1
	x_wid = .9/x_cells
	for i=0,x_cells do begin
		plots,[i*x_wid+.05,i*x_wid+.05],[.06,.3],/normal,thick=3.
	endfor
	for i=1,4 do begin
		plots,[.05,.95],[.3-i*.06,.3-i*.06],/normal,thick=3.
	endfor

	xyouts,.06,.25,'Lag',/normal,charthick=3.
	xyouts,.06,.19,'Pulses',/normal,charthick=3.
	xyouts,.06,.13,'Lag',/normal,charthick=3.
	xyouts,.06,.07,'Pulses',/normal,charthick=3.

	for i=0,nlags-1 do begin
		if(i lt x_cells-1) then row = 0 $
		else row = 1
		;plot the lag numbers
		y = .25-row*.12
		if(row eq 0) then x = .06+x_wid*(i+1) $
		else x = .06+x_wid*(i-x_cells+2)
		xyouts,x,y,strtrim(lag_avail(i),2),/normal,charthick=3.

		;plot the pulses used
		y = .19-row*.12
		xyouts,x,y,strtrim(lagt1(i),2)+','+strtrim(lagt2(i),2),/normal,charthick=3.
	endfor

	oldpage = -1
	x_cells = lag_avail(nlags-1)+1
	x_wid = .9/x_cells
	y_height = .06
	y_cells = .9/y_height
	delta = .001
	for i=0,nrang-1 do begin
		page = i/15
		if(page ne oldpage) then begin
			erase
			for j=0,y_cells do begin
				plots,[.05,.95],[.95-j*y_height,.95-j*y_height],thick=3.,/normal
			endfor
			for j=0,x_cells do begin
				plots,[.05+j*x_wid,.05+j*x_wid],[.05,.95],thick=3.,/normal
			endfor
		endif
		oldpage = page
		col = (i MOD y_cells)
		xyouts,.02,.97-(col+1)*y_height,strtrim(i,2),charthick=3.,/normal
		for j=0,x_cells-1 do begin
			if(col eq 0) then xyouts,.05+x_wid/2.+j*x_wid,.96,strtrim(j,2),align=.5,charthick=3.,/normal
			x1 = .05+j*x_wid+delta
			x2 = .05+(j+1)*x_wid-delta
			y1 = .95-col*y_height-delta
			y2 = .95-(col+1)*y_height+delta
			x = where(lag_avail eq j)
			if(x eq -1) then begin
				polyfill,[x1,x2,x2,x1],[y1,y1,y2,y2],/normal,col=150
				continue
			endif
			loadct,34
			samp1 = lagt1(x)*mpinc+lagfr+i
			samp2 = lagt2(x)*mpinc+lagfr+i
			if(samples(samp1) eq 1 OR samples(samp2) eq 1) then color = 250 $
			else color = 150
			polyfill,[x1,x2,x2,x1],[y1,y1,y2,y2],/normal,col=color
			loadct,0
			xyouts,x1+.003,y2+.035,strtrim(samp1,2),charthick=3.,/normal
			xyouts,x1+.003,y2+.005,strtrim(samp2,2),charthick=3.,/normal
		endfor

	endfor
	;Finally, plot the good/bad lags chart
	


	;close the postscript file
  device,/close
end